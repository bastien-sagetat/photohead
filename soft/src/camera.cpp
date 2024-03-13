/*
 * File: camera.cpp
 * Description: Essentially a libgphoto2 C to C++ wrapper.
 * Created on: 10 December 2023
 */

#include "camera.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace camera
{
    // Context class implementation

    void ContextErrorHandler(GPContext *gp_context, const char *str, void *data)
    {
        (void) (gp_context);

        Context *context = static_cast<Context*>(data);
        context->error_func_(str);
    }

    void ContextStatusHandler(GPContext *gp_context, const char *str, void *data)
    {
        (void) (gp_context);

        Context *context = static_cast<Context*>(data);
        context->status_func_(str);
    }

    Context::Context(ContextErrorFunc error_func, ContextStatusFunc status_func):
        gp_context_(NULL),
        error_func_(NULL),
        status_func_(NULL)
    {
        if ((!error_func) || (!status_func))
        {
            throw CameraException("camera::Context:Context constructor called with NULL parameter(s)");
        }

        gp_context_ = gp_context_new();
        if (!gp_context_)
        {
            throw CameraException("Call to gp_context_new() failed");
        }

        gp_context_set_error_func(gp_context_, ContextErrorHandler, static_cast<void*>(this));
        gp_context_set_status_func(gp_context_, ContextStatusHandler, static_cast<void*>(this));

        // Set attributes
        error_func_  = error_func;
        status_func_ = status_func;
    }

    Context::~Context()
    {
	    gp_context_unref(gp_context_);
    }

    // File class implementation

    File::File():
        gp_file_(NULL)
    {
        int ret = gp_file_new(&gp_file_);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_file_new() failed. error code: " + std::to_string(ret));
        }
    }

    File::~File()
    {
        gp_file_free(gp_file_);
    }

    void File::Save(const std::filesystem::path &file_path) const
    {
        int ret = gp_file_save(gp_file_, file_path.c_str());
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_file_save() failed. error code: " + std::to_string(ret));
        }
    }

    std::string File::Mime() const
    {
	    const char *mime;

        int ret = gp_file_get_mime_type (gp_file_, &mime);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_file_get_mime_type() failed. error code: " + std::to_string(ret));
        }
        return std::string(mime);
    }

    const char* File::Data() const
    {
        const char *data;
        unsigned long int size;
	    int ret = gp_file_get_data_and_size (gp_file_, &data, &size);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_file_get_data_and_size() failed. error code: " + std::to_string(ret));
        }
        return data;
    }

    unsigned long int File::Size() const
    {
        const char *data;
        unsigned long int size;
	    int ret = gp_file_get_data_and_size (gp_file_, &data, &size);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_file_get_data_and_size() failed. error code: " + std::to_string(ret));
        }
        return size;
    }

    // Camera class implementation

    Camera::Camera(const Context &context):
    context_(context),
    gp_camera_(NULL),
    initialized_(false)
    {
        int ret;

        ret = gp_camera_new(&gp_camera_);
	    if (ret < GP_OK)
        {
            throw CameraException("Call to gp_camera_new() failed. error code: " + std::to_string(ret));
        }
    }

    Camera::~Camera()
    {
        if (initialized_)
        {
            gp_camera_exit(gp_camera_, context_.gp_context_);
        }
        gp_camera_free(gp_camera_);
    }

    void Camera::Init()
    {
        int ret = gp_camera_init(gp_camera_, context_.gp_context_);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_camera_init() failed. error code: " + std::to_string(ret));

        }
        initialized_ = true;
    }

    std::string Camera::Summary() const
    {
    	CameraText text;

        if (!initialized_)
        {
            throw CameraException("Call to camera::Camera::Summary() failed because the camera is uninitialized");
        }

        int ret = gp_camera_get_summary(gp_camera_, &text, context_.gp_context_);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_camera_get_summary() failed. error code: " + std::to_string(ret));
        }

        return std::string(text.text);
    }

    void Camera::CapturePreview(const File &file) const
    {
        if (!initialized_)
        {
            throw CameraException("Call to camera::Camera::CapturePreview() failed because the camera is uninitialized");
        }

        int ret = gp_camera_capture_preview(gp_camera_, file.gp_file_, context_.gp_context_);
        if (ret < GP_OK)
        {
            throw CameraException("Call to gp_camera_capture_preview() failed. error code: " + std::to_string(ret));
        }
    }
}
