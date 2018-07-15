#include "camera.h"
#include "cv_config.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <string>

/* ---------------------------- CameraBase ---------------------------*/

std::string get_cam_pipeline(int width, int height, int fps, bool flip, bool auto_exposure) {
    int flip_ = 0;
    if (flip)
        flip_ = 2;
    std::string cam_ctl_command = "";
    if (!auto_exposure) {
        cam_ctl_command += "auto-exposure=1 exposure-time=0.15 aeLock=true";
    }
    return "nvcamerasrc " + cam_ctl_command +
        " ! video/x-raw(memory:NVMM), width=(int)" +
        std::to_string(width) + ", height=(int)" + std::to_string(height) +
        ",format=(string)NV12, framerate=(fraction)" + std::to_string(fps) +
        "/1 ! nvvidconv flip-method=" + std::to_string(flip_) +
        " ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

CameraBase::CameraBase() {
    _buffer.resize(2);
    _write_index = 0;
    _read_index = 1;
}

void CameraBase::set_img(unsigned int sleep) {
    while (true) {
        _buffer[_write_index] = cam_read();
        _lock.lock();
        unsigned short tmp_index;
        tmp_index = _write_index;
        _write_index = _read_index;
        _read_index = tmp_index;
        _lock.unlock();
        if (sleep)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void CameraBase::get_img(Mat &dst) {
    _lock.lock();
    _buffer.at(_read_index).copyTo(dst);
    _lock.unlock();
}

void CameraBase::start() {
    _buffer[_read_index] = cam_read();
    std::thread t(&CameraBase::set_img, this, 0);
    t.detach();
}

/* ---------------------------- SimpleCVCam ---------------------------*/

SimpleCVCam::SimpleCVCam() : CameraBase() {
    // nothing
}

SimpleCVCam::SimpleCVCam(unsigned short device_id) : CameraBase() {
    cap = VideoCapture(device_id);
    start();
}

SimpleCVCam::~SimpleCVCam() {
    cap.release();
}

Mat SimpleCVCam::cam_read() {
    Mat frame;
    cap >> frame;
    return frame;
}

/* ---------------------------- CSICam ---------------------------*/

CSICam::CSICam() : SimpleCVCam() {
    std::string default_pipeline = get_cam_pipeline(IMAGE_WIDTH, IMAGE_HEIGHT, 120, true, false);
    cap = VideoCapture(default_pipeline.c_str());
    start();
}

CSICam::CSICam(char *pipeline) : SimpleCVCam() {
    cap = VideoCapture(pipeline);
    start();
}

/* -------------------------- OV5693Cam ----------------------- */
OV5693Cam::OV5693Cam() : CSICam() {
    std::string DEFAULT_I2C_ = "/dev/i2c-2";
    if (!i2c_init(DEFAULT_I2C_.c_str()))
        std::cerr << "I2C init failed! Continuing anyway...";
}

OV5693Cam::OV5693Cam(char *pipeline, char *i2c_file) : CSICam(pipeline) {
    if(!i2c_init(i2c_file))
        std::cerr << "I2C init failed! Continuing anyway...";
}

bool OV5693Cam::i2c_init(const char *i2c_file) {
    int file;
    bool ret = true;

    if ((file = open(i2c_file, O_RDWR)) < 0) {
        std::cerr << "cannot open i2c file " << i2c_file << std::endl;
        return false;
    }

    if (ioctl(file, I2C_SLAVE, ad5823_addr) < 0) {
        std::cerr << "cannot set i2c slave " << ad5823_addr << std::endl;
        return false;
    }

    i2c_fd = file;

    ret = ret && i2c_write(VCM_MOVE_TIME, 0x43);
    ret = ret && i2c_write(VCM_MODE, 0x00);
    ret = ret && set_focus(default_focus);

    return ret;
}

bool OV5693Cam::i2c_write(uint8_t cmd, uint8_t val) {
    uint8_t data[2];

    data[0] = cmd;
    data[1] = val;

    return write(i2c_fd, data, 2) == 2;
}

bool OV5693Cam::set_focus(uint16_t focus) {
    return i2c_write(VCM_CODE_MSB, 0x04 | (focus >> 8)) &&
        i2c_write(VCM_CODE_LSB, focus & 0xff);
}
