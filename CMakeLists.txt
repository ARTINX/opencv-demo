cmake_minimum_required(VERSION 3.5.0)
project(opencv-demo VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(${CMAKE_TOOLCHAIN_FILE})

find_package(OpenCV CONFIG REQUIRED)

include_directories(include)

file(GLOB_RECURSE SRC_FILES
    "src/*.cpp")

add_executable(opencv-demo ${SRC_FILES})

target_link_libraries(opencv-demo PRIVATE opencv_core opencv_videoio opencv_mcc opencv_calib3d opencv_tracking)

