#ifdef ARTINX_HIK
#include <exception>
#include <map>
#include <string>
#include <cstdint>
#include <thread>
#include <filesystem>
#include <fstream>
#include <glm/glm.hpp>
#include "HikDriver.hpp"
#include "utils.hpp"
#include "CameraParams.h"

#define CheckErrorCode(ERROR_CODE)                                                            \
    {                                                                                         \
        if((ERROR_CODE) != MV_OK) {                                                           \
            logError(fmt::format("Hik Driver error: {:x} at line {}", ERROR_CODE, __LINE__)); \
        }                                                                                     \
    }

/**
 * @brief Constructs a HikDriver object.
 *
 * Initializes the HikDriver with the provided image callback and configuration path.
 * Displays the driver version, reads the configuration file, and opens the camera.
 * If reading the config file fails, logs a warning and sets default exposure time and gain.
 *
 * @param imageCallBack A callback function to process images.
 * @param configPath The path to the configuration file.
 */
HikDriver::HikDriver(std::function<void(cv::Mat&)>& imageCallBack, const std::string& configPath) {
    this->imageCallBack = imageCallBack;
    showDriverVersion();
    try {
        readConfig(configPath);
    } catch (std::exception& e) {
        logWarning(fmt::format("Failed to read config file: {}. Use default exposure time and gain instead.", configPath));
        this->exposureTime = 0.005;
        this->gain = 10.0;
    }
    
    openCamera();
}

HikDriver::~HikDriver() {
    closeCamera();
}
/**
 * @brief read Camera settings from configuration file
 * 
 * @param filePath the file path of the configuration file
 * 
 */
void HikDriver::readConfig(const std::string& filePath) {
    std::ifstream configFile(filePath);
    std::map<std::string, std::string> configMap;

    if (!configFile.is_open()) {
        throw std::runtime_error("Unable to open configuration file: " + filePath);
    }
    logInfo(fmt::format("Opening config file: {}", filePath));
    std::string line;
    while (std::getline(configFile, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = line.substr(0, delimiterPos);
            std::string value = line.substr(delimiterPos + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            configMap[key] = value;
        } else {
            std::cerr << "Skipping invalid line: " << line << std::endl;
            throw std::runtime_error("Invalid configuration file format");
        }
    }
    configFile.close();
    this->exposureTime = std::stod(configMap["exposureTime"]);
    this->gain = std::stod(configMap["gain"]);
}
/**
 * @brief show the Hik driver version
 * 
 */
void HikDriver::showDriverVersion() {
    auto version = MV_CC_GetSDKVersion();
    logInfo(fmt::format("Hik driver version: {}.{}.{}.{}", (version & 0xFF000000) >> 24, (version & 0xFF0000) >> 16,
                        (version & 0xFF00) >> 8, version & 0xFF));
}
/**
 * @brief Opens the Hik camera with specified retry parameters.
 *
 * This function attempts to find and open a single Hik camera device. It retries the connection
 * for a specified number of times with a given interval between retries. Upon successful
 * detection, it initializes the camera handle, configures various camera settings such as
 * exposure, gain, acquisition mode, and balance white auto. It also loads calibration data
 * and starts image grabbing with a BGR callback.
 *
 * @param retryTimes The number of times to retry finding the camera if none are detected.
 * @param retryInterval The interval between each retry attempt.
 */
void HikDriver::openCamera(uint32_t retryTimes, Duration retryInterval) {
    // Enumerate devices until at least one is found
    for(uint32_t i = 0;; ++i) {
        CheckErrorCode(MV_CC_EnumDevices(MV_USB_DEVICE, &mDeviceList));
        if(mDeviceList.nDeviceNum > 0) {
            break;
        } else if(i >= retryTimes) {
            logError(fmt::format(R"(No hik camera found after {} time(s) try)", retryTimes));
            return;
        }
        logInfo(fmt::format("{}th time try to find hik camera, found 0 device", i));
        std::this_thread::sleep_for(retryInterval);
    }
    logInfo(fmt::format("{} camera(s) found", mDeviceList.nDeviceNum));
    
    // Open the first device found
    mDeviceInfo = mDeviceList.pDeviceInfo[0];
    
    // Get camera serial number
    mCameraSerialNumber = std::string(reinterpret_cast<char*>(mDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber));
    
    // Create camera handle
    CheckErrorCode(MV_CC_CreateHandleWithoutLog(&mCameraHandle, mDeviceInfo));
    
    // Don't mention. It's try to solve a bug in the driver. 
    CheckErrorCode(MV_CC_CloseDevice(mCameraHandle));

    CheckErrorCode(MV_CC_OpenDevice(mCameraHandle, MV_ACCESS_Control));
    CheckErrorCode(MV_CC_GetImageInfo(mCameraHandle, &mImageInfo));
    logInfo(fmt::format("Resolution for {}: {} x {}", mCameraSerialNumber, mImageInfo.nWidthMax, mImageInfo.nHeightMax));
    
    loadCalibration(mCameraSerialNumber, mImageInfo.nWidthMax, mImageInfo.nHeightMax);
    
    // Set the properties of Camera, including Exposure time, Gain, AcquisitionMode, BalanceWhite...
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF));
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "ExposureMode", MV_EXPOSURE_MODE_TIMED));
    CheckErrorCode(MV_CC_SetFloatValue(mCameraHandle, "ExposureTime", this->exposureTime * 1.0e6));
    CheckErrorCode(MV_CC_SetFloatValue(mCameraHandle, "Gain", this->gain));
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "AcquisitionMode", MV_ACQ_MODE_CONTINUOUS));
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_OFF));
    CheckErrorCode(MV_CC_SetBayerCvtQuality(mCameraHandle, 1));
    CheckErrorCode(MV_CC_RegisterImageCallBackForBGR(mCameraHandle, newFrame, this));

    // Start grabbing images from the camera
    CheckErrorCode(MV_CC_StartGrabbing(mCameraHandle));
}

/**
 * @brief Handles a new frame received by the HikDriver.
 *
 * This function is called when a new frame is available. It converts the raw frame data into a cv::Mat object
 * in BGR format and invokes the image callback with the processed image.
 *
 * @param pData Pointer to the raw frame data.
 * @param pFrameInfo Pointer to the frame information structure containing metadata about the frame.
 * @param pUser Pointer to the user-defined data, typically a reference to the HikDriver instance.
 */
void HikDriver::newFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    auto driver = static_cast<HikDriver*>(pUser);
    cv::Mat bgr{ pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, static_cast<void*>(pData) };
    driver->imageCallBack(bgr);
}

/**
 * @brief Loads camera calibration data for the specified identifier.
 *
 * @param identifier A unique string identifier for the calibration data (e.g., serial number).
 * @param width The width of the camera image in pixels.
 * @param height The height of the camera image in pixels.
 * @param fallbackFov The fallback field of view (in degrees) to use if calibration data is unavailable.
 */
void HikDriver::loadCalibration(const std::string& identifier, const uint32_t width, const uint32_t height,
                                const double fallbackFov) {
    const auto inputFileName = "./calibration/" + identifier + ".xml";

    const cv::FileStorage fs(inputFileName, cv::FileStorage::READ);
    if(std::filesystem::exists(inputFileName) && fs.isOpened()) {
        fs["camera_matrix"] >> this->cameraMatrix;
        fs["distortion_coefficients"] >> this->distCoefficients;
    } else {
        logWarning(fmt::format("Failed to get calibration info for serial number {}. Use default fov {} instead.", identifier,
                               fallbackFov));
        this->cameraMatrix = (cv::Mat_<double>(3, 3) << width / 2.0 / std::tan(glm::radians(fallbackFov) / 2.0), 0,
                        static_cast<double>(width) / 2.0, 0, height / 2.0 / std::tan(glm::radians(fallbackFov) / 2.0),
                        static_cast<double>(height) / 2.0, 0, 0, 1);
        this->distCoefficients = cv::Mat{};
    }
}
/**
 * @brief Close the camera dirver
 * 
 */
void HikDriver::closeCamera() {
    CheckErrorCode(MV_CC_StopGrabbing(mCameraHandle));
    CheckErrorCode(MV_CC_CloseDevice(mCameraHandle));
    CheckErrorCode(MV_CC_DestroyHandle(mCameraHandle));
}

#endif