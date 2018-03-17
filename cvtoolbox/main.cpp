#include "camera.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <iostream>
#include <atomic>

using namespace std;
using namespace cv;

class cvCam: public CameraBase {
public:
    cvCam() : CameraBase() {
        cap = VideoCapture(0);
        cap >> _buffer[_read_index];
        cap >> _buffer[_write_index];
        std::thread *t = new std::thread(&cvCam::set_img, this);
        //delete t;
    }

    Mat cam_read() {
        Mat frame;
        cap >> frame;
        return frame;
    }
private:
    VideoCapture cap;
};

int main() {
    cvCam cam;
    Mat frame;
    while (true) {
        cam.get_img(frame);
        imshow("go", frame); 
        waitKey(1);
    }
    return 0;
}
