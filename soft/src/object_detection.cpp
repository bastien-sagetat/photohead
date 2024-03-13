/*
 * File: object_detection.c
 * Description: Objection detection
 * Created on: 3 January 2024
 */

#include "object_detection.h"
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>
#include <tensorflow/lite/model.h>
#include <opencv2/opencv.hpp>

namespace object_detection
{
    // Object class

    Object::Object(int label_id, const std::string &label,
                   float score,
                   float x, float y,
                   float width, float height,
                   float center_x, float center_y):
        label_id_(label_id),
        label_(label),
        score_(score),
        x_(x),
        y_(y),
        width_(width),
        height_(height),
        center_x_(center_x),
        center_y_(center_y)
    {}

    // Getter(s)
    int Object::LabelId()       const { return label_id_; }
    std::string Object::Label() const { return label_;    }
    float Object::Score()       const { return score_;    }
    float Object::X()           const { return x_;        }
    float Object::Y()           const { return y_;        }
    float Object::Width()       const { return width_;    }
    float Object::Height()      const { return height_;   }
    float Object::CenterX()     const { return center_x_; }
    float Object::CenterY()     const { return center_y_; }

    // ObjectDetector class

    ObjectDetector::ObjectDetector():
        labels_(),
        input_width_(),
        input_height_(),
        input_channels_(),
        input_type_(),
        objects_(),
        timespan_()
    {}

    void ObjectDetector::BuildModel(const std::filesystem::path &model_path, const std::filesystem::path &labels_path)
    {
        TfLiteStatus status_code;

        // TODO: check that input file paths exist

        model_ = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
        if (!model_)
        {
            throw ObjectDetectionException("Fail to build FlatBufferModel from file: " + model_path.string());
        }

        labels_ = ReadLabels(labels_path);

        tflite::ops::builtin::BuiltinOpResolver resolver;
        status_code = tflite::InterpreterBuilder(*model_, resolver)(&interpreter_);
        if (status_code != kTfLiteOk)
        {
            throw ObjectDetectionException("Fail to build interpreter. errror code: " + std::to_string(status_code));
        }

        status_code = interpreter_->AllocateTensors();
        if (status_code != kTfLiteOk)
        {
            throw ObjectDetectionException("Failed to allocate tensors. errror code: " + std::to_string(status_code));
        }

        const auto& dimensions = interpreter_->tensor(interpreter_->inputs()[0])->dims;
        input_height_ = dimensions->data[1];
        input_width_ = dimensions->data[2];
        input_channels_ = dimensions->data[3];
        input_type_ = interpreter_->tensor(interpreter_->inputs()[0])->type;
    }

    void ObjectDetector::RunInference(const cv::Mat &image, float score_threshold, int num_of_threads)
    {
        // TODO: check that input_type_ == kTfLiteUInt8, input_channels_==3, and image.type() == CV_8UC3

        cv::Mat input_image;
        const auto& start_time = std::chrono::steady_clock::now();

        // Resize the image to the size of the model's input
        cv::resize(image, input_image, cv::Size(Width(), Height()));
        // Convert image colors from BGR to RGB
        // (OpenCV uses BGR color format while most object detection models use RGB color format)
        cv::cvtColor(input_image, input_image, cv::COLOR_BGR2RGB);

        interpreter_->SetNumThreads(num_of_threads);
        //interpreter_->SetAllowFp16PrecisionForFp32(true);

        memcpy(interpreter_->typed_input_tensor<uchar>(0), input_image.data, input_image.total() * input_image.elemSize());

        interpreter_->Invoke();

        const float *locations   =  interpreter_->tensor(interpreter_->outputs()[0])->data.f;
        const float *classes     =  interpreter_->tensor(interpreter_->outputs()[1])->data.f;
        const float *scores      =  interpreter_->tensor(interpreter_->outputs()[2])->data.f;
        const int num_detections = *interpreter_->tensor(interpreter_->outputs()[3])->data.f;

        objects_.clear();
        for(int i = 0; i < num_detections; i++)
        {
            if(scores[i] > score_threshold)
            {
                int  class_idx = (int)classes[i] + 1;
                // Coordinate's origin is at the top-left corner of the image.
                // All coordinates are normalized between 0.0 and 1.0 (inclusive)
                float y0 = locations[4*i];   // y coordinate of the object's top-left corner
                float x0 = locations[4*i+1]; // x coordinate of the object's top-left corner
                float y1 = locations[4*i+2]; // y coordinate of the object's bottom-right corner
                float x1 = locations[4*i+3]; // x coordinate of the object's bottom-right corner

                objects_.push_back(Object(
                    class_idx,                 // label_id
                    labels_[class_idx],        // label
                    scores[i],                 // score
                    x0,                        // x
                    y0,                        // y
                    (x1 - x0),                 // width
                    (y1 - y0),                 // height
                    (x0 + ((x1 - x0) / 2.0f)), // center_x
                    (y0 + ((y1 - y0) / 2.0f))  // center_y
                ));
            }
        }
        timespan_ = std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::steady_clock::now() - start_time).count();
    }

    void ObjectDetector::ApplyOverlay(cv::Mat &image)
    {
        int image_width  = image.cols;
        int image_height = image.rows;

        for (auto &object : objects_)
        {
            // Each object coordinates is normalized (between 0.0 and 1.0).
            // Those coordinates need to be scaled to the image resolution
            int rectangle_x      = (int) (object.X()      * image_width);
            int rectangle_y      = (int) (object.Y()      * image_height);
            int rectangle_width  = (int) (object.Width()  * image_width);
            int rectangle_height = (int) (object.Height() * image_height);

            // Draw a rectangle around each detected object
            cv::Rect rectangle(rectangle_x, rectangle_y, rectangle_width, rectangle_height);
            cv::rectangle(image, rectangle, cv::Scalar(0, 0, 255), 1);

            // Add text depicting each object
            std::ostringstream text;
            text << object.Label() <<  "(" << std::fixed << std::setprecision(2) << object.Score() << ")";
            cv::putText(image, text.str(), cv::Point(rectangle_x, rectangle_y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
        }
        // Add general information text to the image
        cv::putText(image, std::to_string(timespan_) + "ms", cv::Point(10, 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
    }

    std::vector<std::string> ObjectDetector::ReadLabels(const std::filesystem::path &labels_path)
    {
        std::vector<std::string> labels;
        std::string label;
        std::ifstream ifs(labels_path.c_str());

        if(!ifs.is_open())
        {
            throw ObjectDetectionException("Fail to read labels from file: " + labels_path.string());
        }

        while (std::getline(ifs, label))
        {
            if(label.size() > 0)
            {
                labels.push_back(label);
            }
        }
        return labels;
    }

    // Getter(s)
    int ObjectDetector::Width() const                          { return input_width_;    }
    int ObjectDetector::Height() const                         { return input_height_;   }
    int ObjectDetector::Channels() const                       { return input_channels_; }
    int64_t ObjectDetector::Timespan() const                   { return timespan_;       }
    TfLiteType ObjectDetector::InputType() const               { return input_type_;     }
    const std::vector<Object>& ObjectDetector::Objects() const { return objects_;        }
}
