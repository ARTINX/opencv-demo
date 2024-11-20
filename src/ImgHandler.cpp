#include "ImgHandler.hpp"

/**
 * @brief Reads an image from the specified file path.
 *
 * This function loads an image from the given file path and stores it in the provided cv::Mat object.
 *
 * @param path The file path to the image.
 * @param result A reference to a cv::Mat object where the loaded image will be stored.
 */
void ImageHandler::readImg(const std::string& path, cv::Mat& result) const{
    result = cv::imread(path, cv::IMREAD_COLOR);
}

/**
 * @brief Displays the provided image in a window with the specified name.
 *
 * This function creates a window with the given name, displays the provided cv::Mat image,
 * and waits for a key press to close the window.
 *
 * @param img The image to be displayed.
 * @param windowName The name of the window in which the image will be shown.
 */
void ImageHandler::showImg(const cv::Mat& img, const std::string& windowName) const{
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
    cv::imshow(windowName, img);
    cv::waitKey(0);
}