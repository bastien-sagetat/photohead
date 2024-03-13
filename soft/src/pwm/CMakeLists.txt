cmake_minimum_required(VERSION 3.25)

project(pwm LANGUAGES CXX VERSION 0.0.1)

# Set SRC_DIR (source directory path)
set(SRC_DIR ${PROJECT_SOURCE_DIR})
# Set SRCS (list of source filepaths)
set(SRCS
    ${SRC_DIR}/pwm.cpp
)

# Set configuration file (template file)
configure_file(${SRC_DIR}/config.h.in ${SRC_DIR}/config.h)

# Project's header files (.h files) are at the same location than the source files (.cpp files)
include_directories(${SRC_DIR})

add_library(pwm_lib STATIC
    ${SRCS}
)

if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)

    add_executable(${PROJECT_NAME} main.cpp ${SRCS})
    target_link_libraries(${PROJECT_NAME} PRIVATE
    pwm_lib
    )

endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    -g
    -Wall
    -Wextra
    -std=c++17
)

#target_link_libraries(test_pwm PRIVATE
#    rt
#)
