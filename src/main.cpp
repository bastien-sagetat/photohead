/*
 * File: main.cpp
 * Description: object-detection program entry point.
 * Created on: 4 December 2023
 */

#include "camera.h"
#include "camera_exceptions.h"
#include <iostream>
//#include <opencv2/core/mat.hpp>
//#include <opencv2/imgcodecs.hpp>
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
    cv::Mat Image;

    (void) (argc);
    (void) (argv);

    std::cout << "Starting object-detection program" << std::endl;
    std::cout << "Version info:" << std::endl;
    std::cout << "  Git sha1: " << VERSION_HASH << std::endl;
    std::cout << "  Build date: " << VERSION_DATE << std::endl;
    std::cout << std::endl;

    try
    {
        camera.Init();

        std::cout << "Summary:" << std::endl
                  << camera.Summary()  << std::endl;

        camera.CapturePreview(file);

        //file.Save("preview.jpg");

        Image = cv::imdecode(cv::Mat(1, (int)file.Size(), CV_8UC1, (void *)file.Data()), cv::IMREAD_UNCHANGED);

        cv::namedWindow("Camera image", cv::WINDOW_AUTOSIZE);
        if (!(Image.empty()))
        {
            cv::imshow("Camera image", Image);
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
