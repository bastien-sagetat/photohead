cmake_minimum_required(VERSION 3.28)

project(object-detection LANGUAGES CXX VERSION 0.0.1)

find_library(gphoto2_LIB gphoto2 REQUIRED)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# tensorflow-liteConfig.cmake does not seem to do much.
# keep associated find_package() in comment just for the record.
# find_package(tensorflow-lite REQUIRED)

# Set SRC_DIR (source directory path)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
# Set SRCS (list of source filepaths)
set(SRCS
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/camera.cpp
    ${SRC_DIR}/object_detection.cpp
)

# Set configuration file (template file)
configure_file(${SRC_DIR}/config.h.in ${SRC_DIR}/config.h)

# Copy model files (model + labels) to the build directory
configure_file(model/labels_COCO.txt ${CMAKE_CURRENT_BINARY_DIR}/model/labels_COCO.txt COPYONLY)
configure_file(model/MobileNetv1-SSD.tflite ${CMAKE_CURRENT_BINARY_DIR}/model/MobileNetv1-SSD.tflite COPYONLY)

# Project's header files (.h files) are at the same location than the source files (.cpp files)
include_directories(${SRC_DIR})

add_executable(${PROJECT_NAME} ${SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE
    -g
    -Wall
    -Wextra
    -std=c++17
)

# Link shared libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${gphoto2_LIB}
    ${OpenCV_LIBS}
    tensorflow-lite
)
