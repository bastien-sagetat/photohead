/*
 * File: main.cpp
 * Description: Photohead application entry point.
 * Created on: 4 December 2023
 */

#include "config.h"
#include "camera.h"
#include "object_detection.h"
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <csignal>
#include <atomic>
#include <opencv2/highgui.hpp>

#define OPENCV_WINDOW_NAME "Camera image"

/**
 * \brief Indicate whether the process has been requested to be terminated.
 */
std::atomic<bool> termination_requested = false;

/**
 * \brief Process's signal (e.g.: systemd SIGTERM) handler.
 */
static void OnProcessSignal(int signal)
{
    switch (signal)
    {
        case SIGINT:
        case SIGTERM:
        {
            termination_requested = true;
            break;
        }
    }
}

/**
 * \brief Print camera error messages. Should be registered as a camera context callback.
 */
static void OnCameraErrorMessage(const char *message)
{
    std::cerr << std::endl
              << "*** Camera error message ***" << std::endl
              << message << std::endl;
}

/**
 * \brief Print camera status messages. Should be registered as a camera context callback.
 */
static void OnCameraStatusMessage(const char *message)
{
    std::cout << message << std::endl;
}

/**
 * \brief Application entry point.
 */
int main(int argc, char** argv)
{
    camera::Context context(OnCameraErrorMessage, OnCameraStatusMessage);
    camera::Camera camera(context);
    camera::File camera_image_file;
    cv::Mat image;
    object_detection::ObjectDetector object_detector;

    (void) (argc);
    (void) (argv);

    std::cout << "Starting " << PROJECT_NAME <<" application" << std::endl;
    std::cout << "Version : " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;

    try
    {
        // Set handler for the process's signals.
        signal(SIGINT, OnProcessSignal);
        signal(SIGTERM, OnProcessSignal);

        // Initialize the camera
        camera.Init();
        //std::cout << "Summary:" << std::endl << camera.Summary()  << std::endl;

        // Build the object detection model using the given model and labels
        object_detector.BuildModel("model/MobileNetv1-SSD.tflite", "model/labels_COCO.txt");

        // Create the window which will display the image (not opened yet).
        cv::namedWindow(OPENCV_WINDOW_NAME, cv::WINDOW_AUTOSIZE);

        // Acquisition/processing loop.
        while (!termination_requested)
        {
            // Get the camera preview image from the camera.
            // Note that the preview image file is never saved on the filesystem.
            camera.CapturePreview(camera_image_file);

            // Convert the preview image file to an OpenCV image.
            image = cv::imdecode(cv::Mat(1, (int)camera_image_file.Size(), CV_8UC1, (void *)camera_image_file.Data()), cv::IMREAD_UNCHANGED);

            // Run the object detection model on the image.
            object_detector.RunInference(image, SCORE_THRESHOLD_DEFAULT, std::thread::hardware_concurrency());

            // Update the image with the object detector overlay (adds object's labels and rectangle boundaries to the image).
            object_detector.ApplyOverlay(image);

            // Show the image in the OpenCV window.
            if (!(image.empty()))
            {
                cv::imshow(OPENCV_WINDOW_NAME, image);
            }

            // cv::waitKey() renders OpenCV windows (like pending imshow), so it has to be called
            cv::waitKey(5);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << PROJECT_NAME <<" application ended" << std::endl;
    return 0;
}
