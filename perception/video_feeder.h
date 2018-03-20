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
        cap >> _buffer[_read_index];
        cap >> _buffer[_write_index];
        std::thread *t = new std::thread(&VideoFeed::set_img, this, 80);
    }

    Mat cam_read() {
        Mat frame;
        cap >> frame;
        if(frame.empty()){
            //std::cout << "this is the end of the video!!!" << std::endl;
            alive = false;
        }
        //resize(frame, frame, Size(640, 360));
        return frame;
    }

    bool is_alive(void) { return alive; }
private:
    bool alive;
    VideoCapture cap;
};

#endif
