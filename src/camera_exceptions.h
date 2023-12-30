/*
 * File: camera.h
 * Description: Camera custom exceptions
 * Created on: 17 December 2023
 */

#ifndef CAMERA_EXCEPTIONS_H_
#define CAMERA_EXCEPTIONS_H_

#include <stdexcept>

namespace camera
{
    /**
     * \class CameraBaseException
     * Camera exception (base class)
     */
    class Exception : public std::runtime_error
    {
    public:
        Exception(std::string &&message):
            std::runtime_error(message)
        {}
    };

    /**
     * \class ErrorCodeException
     * Camera error code exception
     */
    class ErrorCodeException : public Exception
    {
    public:
        ErrorCodeException(int ret, std::string &&message):
            Exception(std::move(message)),
            result_code_(ret)
        {}

        /**
         * \brief Get the error code received from the gphoto2 function
         */
        int GetResultCode() const
        {
            return result_code_;
        }
    private:
        const int result_code_;
    };
}

#endif // !defined(CAMERA_EXCEPTIONS_H_)
