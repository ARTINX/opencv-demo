#ifdef ARTINX_HIK
#pragma once
#include <chrono>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <MvCameraControl.h>
#include <MvErrorDefine.h>
#include <functional>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

using Clock = std::chrono::steady_clock;
using Duration = Clock::duration;
using std::literals::chrono_literals::operator""ms;

class HikDriver final {
private: 
    void* mCameraHandle;
    cv::Mat cameraMatrix;
    cv::Mat distCoefficients;
    std::string mCameraSerialNumber;
    std::function<void(cv::Mat&)> imageCallBack;
    MV_CC_DEVICE_INFO_LIST mDeviceList;
    MV_CC_DEVICE_INFO* mDeviceInfo;
    MV_IMAGE_BASIC_INFO mImageInfo;

    double exposureTime;
    double gain;

    void showDriverVersion();
    void readConfig(const std::string& filePath);
    void openCamera(uint32_t retryTimes = 1, Duration retryInterval = 0ms);
    static void newFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
    void loadCalibration(const std::string& identifier, const uint32_t width, const uint32_t height,
                                const double fallbackFov = 30.0);
    void closeCamera();


public:
    HikDriver(std::function<void(cv::Mat&)>& imageCallBack, const std::string& configPath);
    ~HikDriver();
};

#endif