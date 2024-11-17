#include <iostream>
#include <opencv2/opencv.hpp>

#include "ImgHandler.hpp"

int main(int, char**){
    cv::Mat src;
    ImageHandler imgHandler;
    imgHandler.readImg("./img/image-demo.png", src);
    imgHandler.showImg(src, "Code");
    return 0;
}
