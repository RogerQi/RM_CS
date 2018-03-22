#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <mutex>
#include <vector>
#include <thread>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

/**
 * A base class for generalized camera level processing
 */
class CameraBase {
public:

    /**
     * @brief constructor for CameraBase class
     */
    CameraBase();

    /**
     * @brief read an image and copy into src Mat
     * @param dst destination place holder for the stored image
     * @return none
     */
    void get_img(Mat &dst);

    /**
     * @brief excecuted in a seperate thread to increase frame rate
     *        and to allow multi camera processing
     * @return none
     */
    void set_img(unsigned int sleep = 0);

    /**
     * @brief start the camera in a sperate thread
     * @return none
     */
    void start();
    /**
     * @brief to be implemented according to the specs of a specific camera
     * @return Mat object read directly from the camera
     */
    virtual Mat cam_read() = 0;

    /**
     * @brief determine if the camera is dead or alive. Should only be of use for video feed
     * @return alive flag
     */
    virtual bool is_alive(void) { return true; }
protected:
    unsigned short _write_index;
    unsigned short  _read_index;
    vector<Mat> _buffer;
    mutex _lock;
};

class SimpleCVCam: public CameraBase {
public:
    SimpleCVCam() : CameraBase() {
        cap = VideoCapture(0);
        start();
    }

    Mat cam_read() {
        Mat frame;
        cap >> frame;
        return frame;
    }
private:
    VideoCapture cap;
};

#endif
