/*
 * File: camera.cpp
 * Description: Essentially a libgphoto2 C to C++ wrapper.
 * Created on: 10 December 2023
 */

#include "camera.h"
#include "camera_exceptions.h"
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
            throw camera::Exception("camera::Context:Context constructor called with NULL parameter(s)");
        }

        gp_context_ = gp_context_new();
        if (!gp_context_)
        {
            throw camera::Exception("Call to gp_context_new() failed");
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
            throw camera::ErrorCodeException(ret, "Call to gp_file_new() failed");
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
            throw camera::ErrorCodeException(ret, "Call to gp_file_save() failed");
        }
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
            throw camera::ErrorCodeException(ret, "Call to gp_camera_new() failed");
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
            throw camera::ErrorCodeException(ret, "Call to gp_camera_init() failed");
        }
        initialized_ = true;
    }

    std::string Camera::Summary() const
    {
    	CameraText text;

        if (!initialized_)
        {
            throw camera::Exception("Call to camera::Camera::Summary() failed because the camera is uninitialized");
        }

        int ret = gp_camera_get_summary(gp_camera_, &text, context_.gp_context_);
        if (ret < GP_OK)
        {
            throw camera::ErrorCodeException(ret, "Call to gp_camera_get_summary() failed");
        }

        return std::string(text.text);
    }

    void Camera::CapturePreview(const File &file) const
    {
        if (!initialized_)
        {
            throw camera::Exception("Call to camera::Camera::CapturePreview() failed because the camera is uninitialized");
        }

        int ret = gp_camera_capture_preview(gp_camera_, file.gp_file_, context_.gp_context_);
        if (ret < GP_OK)
        {
            throw camera::ErrorCodeException(ret, "Call to gp_camera_capture_preview() failed");
        }
    }
}
