#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <mutex>
#include <vector>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/i2c-dev.h>

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
    std::vector<cv::Mat> _buffer;
    mutex _lock;
};

/**
 * @brief a generalized opencv camera class
 */
class SimpleCVCam: public CameraBase {
public:
    SimpleCVCam();
    /**
     * @brief constructor for SimpleCVCam
     * @param device_id device id as in camera index [default to 0]
     */
    SimpleCVCam(unsigned short device_id);

    ~SimpleCVCam();

    /**
     * @brief   virtual implementation of how to read images
     * @return  Mat object read directly from the camera
     */
    Mat cam_read();
protected:
    VideoCapture cap;
};

/**
 * @brief a generalized CSI camera class
 */
class CSICam: public SimpleCVCam {
public:
    CSICam();
    /**
     * @brief constructor for CSICam
     * @param pipeline Gstreamer pipeline
     */
    CSICam(char *pipeline);
};


/**
 * @brief a specialized camera class for variable focal length OV5693
 */
class OV5693Cam: public CSICam {
public:
    OV5693Cam();

    /**
     * @brief constructor for OV5693Cam
     * @param pipeline Gstreamer pipeline
     * @param i2c_file
     */
    OV5693Cam(char *pipeline, char *i2c_file);

    /**
     * @brief change camera focal length
     * @param focus 10 bit focal length value within [0 1023] (0 being the longest focal length)
     * @return
     */
    bool set_focus(uint16_t focus);

private:
    bool    i2c_init(const char *i2c_file);
    bool    i2c_write(uint8_t cmd, uint8_t val);

    int i2c_fd = -1;
    static const uint16_t   default_focus   = 100;
    static const uint16_t   ad5823_addr     = 0x0c;
    static const uint8_t    VCM_MOVE_TIME   = 0x03;
    static const uint8_t    VCM_MODE        = 0x02;
    static const uint8_t    VCM_CODE_MSB    = 0x04;
    static const uint8_t    VCM_CODE_LSB    = 0x05;
};


#endif
