#ifdef ARTINX_HIK
#include <cstdint>
#include <thread>
#include <filesystem>
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

HikDriver::HikDriver(std::function<void(cv::Mat&)>& imageCallBack) {
    this->exposureTime = 0.005;
    this->gain = 10.0;
    this->imageCallBack = imageCallBack;
    showDriverVersion();
    openCamera();
}

HikDriver::~HikDriver() {
    closeCamera();
}

void HikDriver::showDriverVersion() {
    auto version = MV_CC_GetSDKVersion();
    logInfo(fmt::format("Hik driver version: {}.{}.{}.{}", (version & 0xFF000000) >> 24, (version & 0xFF0000) >> 16,
                        (version & 0xFF00) >> 8, version & 0xFF));
}

void HikDriver::openCamera(uint32_t retryTimes, Duration retryInterval) {
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
    mDeviceInfo = mDeviceList.pDeviceInfo[0];
    mCameraSerialNumber = std::string(reinterpret_cast<char*>(mDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber));
    CheckErrorCode(MV_CC_CreateHandleWithoutLog(&mCameraHandle, mDeviceInfo));
    CheckErrorCode(MV_CC_CloseDevice(mCameraHandle));
    CheckErrorCode(MV_CC_OpenDevice(mCameraHandle, MV_ACCESS_Control));
    CheckErrorCode(MV_CC_GetImageInfo(mCameraHandle, &mImageInfo));
    logInfo(fmt::format("Resolution for {}: {} x {}", mCameraSerialNumber, mImageInfo.nWidthMax, mImageInfo.nHeightMax));
    loadCalibration(mCameraSerialNumber, mImageInfo.nWidthMax, mImageInfo.nHeightMax);
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF));
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "ExposureMode", MV_EXPOSURE_MODE_TIMED));
    CheckErrorCode(MV_CC_SetFloatValue(mCameraHandle, "ExposureTime", this->exposureTime * 1.0e6));
    CheckErrorCode(MV_CC_SetFloatValue(mCameraHandle, "Gain", this->gain));
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "AcquisitionMode", MV_ACQ_MODE_CONTINUOUS));
    CheckErrorCode(MV_CC_SetEnumValue(mCameraHandle, "BalanceWhiteAuto", MV_BALANCEWHITE_AUTO_OFF));
    CheckErrorCode(MV_CC_SetBayerCvtQuality(mCameraHandle, 1));
    CheckErrorCode(MV_CC_RegisterImageCallBackForBGR(mCameraHandle, newFrame, this));
    CheckErrorCode(MV_CC_StartGrabbing(mCameraHandle));
}

void HikDriver::newFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    auto driver = static_cast<HikDriver*>(pUser);
    cv::Mat bgr{ pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, static_cast<void*>(pData) };
    driver->imageCallBack(bgr);
}

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

void HikDriver::closeCamera() {
    CheckErrorCode(MV_CC_StopGrabbing(mCameraHandle));
    CheckErrorCode(MV_CC_CloseDevice(mCameraHandle));
    CheckErrorCode(MV_CC_DestroyHandle(mCameraHandle));
}

#endif