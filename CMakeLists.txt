cmake_minimum_required(VERSION 3.28)

project(object-detection LANGUAGES CXX VERSION 0.0.1)

#TODO: Add flags (-g -Wall -Wextra -std=c++17)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(SRCS
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/camera.cpp
)

include_directories(${SRC_DIR})

add_executable(${PROJECT_NAME} ${SRCS})
