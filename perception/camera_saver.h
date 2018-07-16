#ifndef CAMERA_SAVER_H_
#define CAMERA_SAVER_H_

#include <thread>
#include <string>
#include <opencv2/opencv.hpp>
#include "cv_config.h"
#include "camera.h"

class video_saver{
public:
    video_saver(CameraBase *_cam, unsigned int interval_ms);

    ~video_saver();
private:
    int save_counter = 0;

    unsigned int sleep_ms;

    std::string save_path_prefix = "/home/nvidia/Pictures/";

    CameraBase *my_cam;

    void save_image(void);

    void start(void);

    void interval_saver(unsigned int sleep_sec);

};

#endif
