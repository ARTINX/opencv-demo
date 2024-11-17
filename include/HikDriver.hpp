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
    void openCamera(uint32_t retryTimes = 1, Duration retryInterval = 0ms);
    static void newFrame(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
    void loadCalibration(const std::string& identifier);
    void closeCamera();


public:
    HikDriver(const std::string& Identifier, std::function<void(cv::Mat&)>& imageCallBack);
    ~HikDriver();
};

#endif