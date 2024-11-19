cmake_minimum_required(VERSION 3.5.0)
project(opencv-demo VERSION 0.1.0 LANGUAGES C CXX)

# clangd need compile_commands.json to analyse dependency
# after adding following line, a compile_commands.json will be generated in build folder
# you need to make a symlink to project's root path.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# check opencv dependency
find_package(OpenCV CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# tell cmake where to find header files
include_directories(include)

# set SRC_FILE to all cpp files in src folder
file(GLOB_RECURSE SRC_FILES
    "src/*.cpp",
    "main.cpp")

# tell cmake which file to compile
add_executable(opencv-demo ${SRC_FILES})

# link opencv libs
target_link_libraries(opencv-demo PRIVATE opencv_core opencv_videoio opencv_mcc opencv_calib3d opencv_tracking)
target_link_libraries(opencv-demo PRIVATE glm::glm)

# add HIK camera SDK
if (DEFINED ENV{HIK_SDK})
    target_include_directories(opencv-demo PRIVATE $ENV{HIK_SDK}/include)
    target_link_directories(opencv-demo PRIVATE $ENV{HIK_SDK}/lib/64)
    target_link_libraries(
            opencv-demo PRIVATE MVGigEVisionSDK FormatConversion MvCameraControl
            MediaProcess MVRender MvUsb3vTL)
    add_compile_definitions("ARTINX_HIK")
    message("USB3: Hik SDK found at $ENV{HIK_SDK}")
endif()

# Explaination: 
# path define in main.cpp is path reference to executable file, not prject's root path
# so we create symlink to make executable file can find img 
add_custom_command(
        TARGET opencv-demo
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/img
        $<TARGET_FILE_DIR:opencv-demo>/img
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/calibration
        $<TARGET_FILE_DIR:opencv-demo>/calibration
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/config
        $<TARGET_FILE_DIR:opencv-demo>/config
)

