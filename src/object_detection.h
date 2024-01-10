/*
 * File: object_detection.h
 * Description: Objection detection
 * Created on: 3 January 2024
 */

#ifndef OBJECT_DETECTION_H_
#define OBJECT_DETECTION_H_

#include <memory>
#include <stdint.h>
#include <string>
#include <filesystem>
#include <exception>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/model.h>
#include <opencv2/opencv.hpp>

#define SCORE_THRESHOLD_DEFAULT (0.5f)

namespace object_detection
{
    /**
     * \class Object
     * Aggregate attibrutes of a single detected object (e.g.: coordinates, label)
     */
    class Object
    {
    public:
        Object(int label_id, const std::string &label,
               float score,
               float x, float y,
               float width, float height,
               float center_x, float center_y);
        // Getter(s)
        int LabelId() const;
        std::string Label() const;
        float Score() const;
        float X() const;
        float Y() const;
        float Width() const;
        float Height() const;
        float CenterX() const;
        float CenterY() const;

    private:
        const int label_id_;      // label's ID in the list of labels
        const std::string label_; // label of the object (e.g.: "person")
        const float score_;
        // All below values of normalized values, between 0.0 and 1.0 (inclusive)
        const float x_;           // x coordinate of the object's top-left corner
        const float y_;           // y coordinate of the object's top-left corner
        const float width_;       // Width of the object
        const float height_;      // Heigth of the object
        const float center_x_;    // x coordinate of the object's center
        const float center_y_;    // y coordinate of the object's center
    };

    /**
     * \class ObjectDetector
     * Build and run the object detection model.
     * Once the object detection model has been run, the detected objects can be retrieved using the appropriate getter function.
     */
    class ObjectDetector
    {
    public:
        ObjectDetector(const float score_threshold);
        /**
		 * \brief Build the object detection model.
         */
        void BuildModel(const std::filesystem::path &model_path, const std::filesystem::path &labels_path);
        /**
		 * \brief Run the object detection model on the given image.
         */
        void RunInference(const cv::Mat &image, int num_of_threads);
        /**
		 * \brief Apply an overlay on the given image to add some info on it (e.g.: detected object's rectangle boundaries).
         */
        void ApplyOverlay(cv::Mat &image);
        // Getter(s)
        int Width() const;
        int Height() const;
        int Channels() const;
        int64_t Timespan() const; // Inference timestamp in ms
        TfLiteType InputType() const;
        const std::vector<Object>& Objects() const;

    private:
        const float score_threshold_;
        std::vector<std::string> labels_;
        std::unique_ptr<tflite::FlatBufferModel> model_;
        std::unique_ptr<tflite::Interpreter> interpreter_;
        int         input_width_;
        int         input_height_;
        int         input_channels_;
        TfLiteType  input_type_;
        std::vector<Object> objects_;
        int64_t timespan_;
        static std::vector<std::string> ReadLabels(const std::filesystem::path &labels_path);
        // Disable copy
        ObjectDetector(ObjectDetector const&) = delete;
        void operator=(ObjectDetector const&) = delete;
    };

    /**
     * \class ObjectDetectionException
     * Object detection exception
     */
    class ObjectDetectionException : public std::runtime_error
    {
    public:
        ObjectDetectionException(std::string &&message):
            std::runtime_error(message)
        {}
    };
}

#endif // !defined(OBJECT_DETECTION_H_)
