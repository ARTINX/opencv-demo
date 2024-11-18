#include <functional>
#include <opencv2/opencv.hpp>

#include "HikDriver.hpp"

int main(int, char**){
    auto imageCallBack_ = [](cv::Mat& image) {
        cv::imshow("Hik camera", image);
        cv::waitKey(1);
    };
    std::function<void(cv::Mat&)> imageCallBack = imageCallBack_;
    HikDriver driver(imageCallBack);
    while (true) {}
    return 0;
}
