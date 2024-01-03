/*
 * File: main.cpp
 * Description: object-detection program entry point.
 * Created on: 4 December 2023
 */

#include "config.h"
#include "camera.h"
#include "camera_exceptions.h"
#include "object_detection.h"
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <opencv2/highgui.hpp>

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
 * \brief Program entry point.
 */
int main(int argc, char** argv)
{
    camera::Context context(OnCameraErrorMessage, OnCameraStatusMessage);
    camera::Camera camera(context);
    camera::File file;
    cv::Mat image;
    object_detection::ObjectDetector object_detector(SCORE_THRESHOLD_DEFAULT);

    (void) (argc);
    (void) (argv);

    std::cout << "Starting object-detection program" << std::endl;
    std::cout << "Version : " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;

    try
    {
        camera.Init();

        //std::cout << "Summary:" << std::endl << camera.Summary()  << std::endl;

        object_detector.BuildModel("model/MobileNetv1-SSD.tflite", "model/labels_COCO.txt");

        camera.CapturePreview(file);

        image = cv::imdecode(cv::Mat(1, (int)file.Size(), CV_8UC1, (void *)file.Data()), cv::IMREAD_UNCHANGED);

        object_detector.RunInference(image, std::thread::hardware_concurrency());

        object_detector.ApplyOverlay(image);

        cv::namedWindow("Camera image", cv::WINDOW_AUTOSIZE);
        if (!(image.empty()))
        {
            cv::imshow("Camera image", image);
        }

        cv::waitKey(0);
    }
    catch (const camera::ErrorCodeException &e)
    {
        std::cerr << "Camera error: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.GetResultCode() << std::endl;
    }
    catch (const camera::Exception &e)
    {
        std::cerr << "Camera error: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::cout << "object-detection program ended" << std::endl;
    return 0;
}
