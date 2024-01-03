cmake_minimum_required(VERSION 3.28)

project(object-detection LANGUAGES CXX VERSION 0.0.1)

find_library(gphoto2_LIB gphoto2 REQUIRED)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# tensorflow-liteConfig.cmake does not seem to do much.
# keep associated find_package() in comment just for the record.
# find_package(tensorflow-lite REQUIRED)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(SRCS
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/camera.cpp
)

configure_file(${SRC_DIR}/config.h.in ${SRC_DIR}/config.h)

configure_file(model/labels_COCO.txt ${CMAKE_CURRENT_BINARY_DIR}/model/labels_COCO.txt COPYONLY)
configure_file(model/MobileNetv1-SSD.tflite ${CMAKE_CURRENT_BINARY_DIR}/model/MobileNetv1-SSD.tflite COPYONLY)

include_directories(${SRC_DIR})

add_executable(${PROJECT_NAME} ${SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE
    -g
    -Wall
    -Wextra
    -std=c++17
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${gphoto2_LIB}
    ${OpenCV_LIBS}
    tensorflow-lite
)
