#pragma once

#include <string>
#include <opencv2/opencv.hpp>

class ImageHandler{
public:
    void readImg(const std::string& path, cv::Mat& result) const;
    void showImg(const cv::Mat& img, const std::string& windowName) const;
};