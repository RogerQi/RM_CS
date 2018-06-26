#include "camera.h"
#include <thread>
#include <chrono>
#include <iostream>

/* ---------------------------- CameraBase ---------------------------*/

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

SimpleCVCam::SimpleCVCam(unsigned short device_id) : CameraBase() {
    cap = VideoCapture(device_id);
    start();
}

Mat SimpleCVCam::cam_read() {
    Mat frame;
    cap >> frame;
    return frame;
}

/* ---------------------------- CSICam ---------------------------*/

CSICam::CSICam(char *pipeline) {
    cap = VideoCapture(pipeline);
    start();
}

/* -------------------------- OV5693Cam ----------------------- */


OV5693Cam::OV5693Cam(char *pipeline, char *i2c_file) : CSICam(pipeline) {
    i2c_init(i2c_file);
}

bool OV5693Cam::i2c_init(char *i2c_file) {
    int file;

    if ((file = open(i2c_file, O_RDWR)) < 0) {
        std::cerr << "cannot open i2c file " << i2c_file << std::endl;
        return false;
    }

#ifdef USE_GPU
    if (ioctl(file, I2C_SLAVE, ad5823_addr) < 0) {
        std::cerr << "cannot set i2c slave " << ad5823_addr << std::endl;
        return false;
    }
#endif

    i2c_fd = file;

    i2c_write(VCM_MOVE_TIME, 0x43);
    i2c_write(VCM_MODE, 0x00);
    set_focus(default_focus);

    return true;
}

bool OV5693Cam::i2c_write(uint8_t cmd, uint8_t val) {
    uint8_t data[2];

    data[0] = cmd;
    data[1] = val;

    return write(i2c_fd, data, 2) == 2;
}

bool OV5693Cam::set_focus(uint16_t focus) {
    i2c_write(VCM_CODE_MSB, 0x04 | (focus >> 8));
    i2c_write(VCM_CODE_LSB, focus & 0xff);
}
