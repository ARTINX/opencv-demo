#include "ImgHandler.hpp"

void ImageHandler::readImg(const std::string& path, cv::Mat& result) const{
    result = cv::imread(path, cv::IMREAD_COLOR);
}

void ImageHandler::showImg(const cv::Mat& img, const std::string& windowName) const{
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    cv::imshow(windowName, img);
    cv::waitKey(0);
}