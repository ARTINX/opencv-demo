#include <functional>
#include <opencv2/opencv.hpp>

#include "HikDriver.hpp"

int main(int argc, char** argv){
    if (argc != 2) {
        std::cerr << "Usage: [program] [config file path]" << std::endl;
        return 1;
    }
    auto imageCallBack_ = [](cv::Mat& image) {
        cv::imshow("Hik camera", image);
        cv::waitKey(1);
    };
    std::function<void(cv::Mat&)> imageCallBack = imageCallBack_;
    HikDriver driver(imageCallBack, argv[1]);
    while (true) {}
    return 0;
}
