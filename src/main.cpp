/*
 * File: main.cpp
 * Description: object-detection program entry point.
 * Created on: 4 December 2023
 */

#include "config.h"
#include "camera.h"
#include "camera_exceptions.h"
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/ocl.hpp>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/model.h"

std::vector<std::string> Labels;
std::unique_ptr<tflite::Interpreter> interpreter;

static bool getFileContent(std::string fileName)
{
	std::ifstream in(fileName.c_str());
	if(!in.is_open()) return false;

	std::string str;
	while (std::getline(in, str))
	{
		if(str.size()>0) Labels.push_back(str);
	}
	in.close();
	return true;
}

void detect_from_video(cv::Mat &src)
{
    cv::Mat image;

    cv::resize(src, image, cv::Size(300,300));
    memcpy(interpreter->typed_input_tensor<uchar>(0), image.data, image.total() * image.elemSize());

    interpreter->SetAllowFp16PrecisionForFp32(true);
    interpreter->SetNumThreads(4);

    interpreter->Invoke();

    const float* detection_classes=interpreter->tensor(interpreter->outputs()[1])->data.f;
    const float* detection_scores = interpreter->tensor(interpreter->outputs()[2])->data.f;
    const int    num_detections = *interpreter->tensor(interpreter->outputs()[3])->data.f;

    const float confidence_threshold = 0.5;
    for(int i = 0; i < num_detections; i++)
    {
        if(detection_scores[i] > confidence_threshold)
        {
            int  det_index = (int)detection_classes[i]+1;
            std::cout << "object: " <<  Labels[det_index].c_str() << std::endl;
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
    std::cout << "Version : " << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;

    try
    {
        camera.Init();

        //std::cout << "Summary:" << std::endl << camera.Summary()  << std::endl;

        std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile("MobileNetv1-SSD.tflite");
        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder(*model.get(), resolver)(&interpreter);
        interpreter->AllocateTensors();

        bool result = getFileContent("labels_COCO.txt");
        if(!result)
        {
            std::cout << "loading labels failed" << std::endl;
            exit(-1);
        }

        camera.CapturePreview(file);

        //file.Save("preview.jpg");

        Image = cv::imdecode(cv::Mat(1, (int)file.Size(), CV_8UC1, (void *)file.Data()), cv::IMREAD_UNCHANGED);

        cv::namedWindow("Camera image", cv::WINDOW_AUTOSIZE);
        if (!(Image.empty()))
        {
            cv::imshow("Camera image", Image);
        }

        detect_from_video(Image);

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
