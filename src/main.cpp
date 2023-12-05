/*
 * File: main.cpp
 * Description: object-detection program entry point.
 * Created on: 4 December 2023
 */

#include <iostream>
#include <gphoto2/gphoto2.h>

extern "C"
{
    int  gp_camera_new(Camera **camera);
    void gp_context_set_error_func(GPContext *context, GPContextErrorFunc func, void *data);
    void gp_context_set_status_func(GPContext *context, GPContextStatusFunc func, void *data);
    int  gp_camera_new(Camera **camera);
    int  gp_camera_init(Camera *camera, GPContext *context);
    int  gp_camera_free(Camera *camera);
    int  gp_camera_exit(Camera *camera, GPContext *context);
    void gp_context_unref(GPContext *context);
    int  gp_camera_get_summary(Camera *camera, CameraText *summary, GPContext *context);
    int  gp_file_new(CameraFile **file);
    int  gp_camera_capture_preview(Camera *camera, CameraFile *file, GPContext *context);
    int  gp_file_save(CameraFile *file, const char *filename);
    int  gp_file_unref(CameraFile *file);
    int  gp_file_free(CameraFile *file);
}

static void ctx_error_func (GPContext *context, const char *str, void *data)
{
    (void) (context);
    (void) (data);

    std::cerr << std::endl
              << "*** Contexterror ***              " << std::endl
              << str << std::endl;
}

static void ctx_status_func (GPContext *context, const char *str, void *data)
{
    (void) (context);
    (void) (data);

    std::cout << str << std::endl;
}

int main(int argc, char** argv)
{
	int ret;
    Camera *camera;
    GPContext *context;
	CameraText text;
    CameraFile *file;

    (void) (argc);
    (void) (argv);

    std::cout << "Starting object-detection program" << std::endl;
    std::cout << "Version info:" << std::endl;
    std::cout << "  Git sha1: " << VERSION_HASH << std::endl;
    std::cout << "  Build date: " << VERSION_DATE << std::endl;
    std::cout << std::endl;

	context = gp_context_new();
    gp_context_set_error_func(context, ctx_error_func, NULL);
    gp_context_set_status_func(context, ctx_status_func, NULL);

	gp_camera_new (&camera);
	// This call will autodetect cameras,
	// take the first one from the list and use it.
	ret = gp_camera_init(camera, context);

	if (ret < GP_OK)
    {
		std::cout << "No camera auto detected." << std::endl;
		gp_camera_free(camera);
		return 1;
	}

	// Query the camera summary text
	ret = gp_camera_get_summary(camera, &text, context);
	if (ret < GP_OK)
    {
		printf("Camera failed retrieving summary.\n");
		gp_camera_free (camera);
		return 1;
	}
    std::cout << "Summary:" << std::endl
              << text.text  << std::endl;

    // TODO: get the camera config ? or just a config subset (just a few variables) ?

    // Capture a preview
    // Saved as preview.jpg
    ret = gp_file_new(&file);
    if (ret != GP_OK)
    {
        std::cerr << "gp_file_new: " << ret << std::endl;
		return 1;
    }

    ret = gp_camera_capture_preview(camera, file, context);
    if (ret != GP_OK)
    {
        std::cerr << "gp_camera_capture_preview failed: " << ret << std::endl;
		return 1;
    }

    // TODO: check mime type and name

    ret = gp_file_save(file, "preview.jpg");
    if (ret != GP_OK)
    {
        std::cerr << "saving preview failed: " << ret << std::endl;
		return 1;
    }
    //gp_file_unref(file);
    gp_file_free(file);

	gp_camera_exit(camera, context);
	gp_camera_free(camera);
	gp_context_unref(context);
    std::cout << "object-detection program ended" << std::endl;
    return 0;
}
