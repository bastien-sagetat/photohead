/*
 * File: camera.h
 * Description: Essentially a libgphoto2 C to C++ wrapper.
 * Created on: 10 December 2023
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <gphoto2/gphoto2.h>
#include <string>
#include <filesystem>
#include <exception>

namespace camera
{
    typedef void (* ContextErrorFunc) (const char *text);
    typedef void (* ContextStatusFunc)(const char *text);
    typedef struct _Camera GPCamera;

    /**
     * \class Context
     * Camera context. Provides callbacks for error and status handling.
     */
    class Context
    {
    public:
        Context(ContextErrorFunc error_func, ContextStatusFunc status_func);
        ~Context();
    private:
        GPContext *gp_context_;
        ContextErrorFunc error_func_;
        ContextStatusFunc status_func_;
        Context(Context const&)        = delete;
        void operator=(Context const&) = delete;
        friend void ContextErrorHandler(GPContext *gp_context, const char *str, void *data);
        friend void ContextStatusHandler(GPContext *gp_context, const char *str, void *data);
        friend class Camera;
    };

    /**
     * \class File
     * Camera file.
     */
    class File
    {
    public:
        File();
        ~File();
        /**
		 * \brief Save a camera file (most probably an image) to the local file system
         */
        void Save(const std::filesystem::path &file_path) const;
        /**
		 * \brief Get file mime type (e.g.: image/jpeg)
         */
        std::string Mime() const;
        /**
		 * \brief Get file data
         */
        const char* Data() const;
        /**
		 * \brief Get file data size
         */
        unsigned long int Size() const;


    private:
        CameraFile *gp_file_;
        File(File const&)           = delete;
        void operator=(File const&) = delete;
        friend class Camera;
    };

    /**
     * \class Camera
     * Camera. Provides camera functions to take pictures, or to get a preview for instance.
     */
    class Camera
    {
    public:
        Camera(const Context &context);
        ~Camera();
        /**
		 * \brief Initialize the connected camera. Should be called first.
         */
        void Init();
        /**
		 * \brief Gets the summary of the connected camera.
         */
        std::string Summary() const;
        /**
		 * \brief Capture a preview image from the camera.
         */
        void CapturePreview(const File &file) const;

    private:
        const Context &context_;
        GPCamera *gp_camera_;
        bool initialized_;

        // Disable copy
        Camera(Camera const&)         = delete;
        void operator=(Camera const&) = delete;
    };

    /**
     * \class CameraException
     * Camera exception
     */
    class CameraException : public std::runtime_error
    {
    public:
        CameraException(std::string &&message):
            std::runtime_error(message)
        {}
    };
}

#endif // !defined(CAMERA_H_)
