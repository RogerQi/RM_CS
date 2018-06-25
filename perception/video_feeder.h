#ifndef _VIDEO_FEEDER_H_
#define _VIDEO_FEEDER_H_

#include "camera.h"
#include <string>
#include <mutex>
#include <vector>
#include <thread>
#include <opencv2/opencv.hpp>
#include <iostream>

using std::string;
using namespace cv;

class VideoFeed: public CameraBase {
public:
    VideoFeed(string video_file_name) : CameraBase() {
        alive = true;
        cap = VideoCapture(video_file_name);
        cap >> next_frame; // read first frame for init.
        //start();
    }

    Mat cam_read() {
        Mat cur_frame = next_frame;
        cap >> next_frame;
        if(next_frame.empty()){
            //std::cout << "this is the end of the video!!!" << std::endl;
            alive = false;
        }
        //resize(frame, frame, Size(640, 360));
        return cur_frame;
    }

    bool is_alive(void) { return alive; }
private:
    Mat next_frame;
    bool alive;
    VideoCapture cap;
};

#endif
