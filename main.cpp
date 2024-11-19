#include <functional>
#include <opencv2/opencv.hpp>

#include "HikDriver.hpp"

int main(int argc, char** argv){
    // 检查参数个数是否为1
    if (argc != 2) {
        std::cerr << "Usage: [program] [config file path]" << std::endl;
        return 1;
    }

    // 回调函数(imageCallBack)在相机捕获到一帧图像时被调用
    // 
    auto imageCallBack_ = [](cv::Mat& image) {
        cv::imshow("Hik camera", image);
        cv::waitKey(1);
    };
    std::function<void(cv::Mat&)> imageCallBack = imageCallBack_;

    /* 
    驱动用法:
    使用一个回调函数与配置文件路径构造HikDriver类
    HikDriver会扫描并连接可用的相机，并按照序列号从calibration文件夹下读取相机内参文件。
    相机会自动开始捕获图像，并在每次捕获到图像的时候调用imageCallBack函数
    */
    HikDriver driver(imageCallBack, argv[1]);
    while (true) {}
    return 0;
}
