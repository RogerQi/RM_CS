#include "camera.h"
#include <thread>
#include <iostream>

CameraBase::CameraBase() {
    _buffer.resize(2);
    _write_index = 0;
    _read_index = 0;
}

void CameraBase::set_img() {
    while (true) {
        _buffer[_write_index] = cam_read(); 
        _lock.lock();
        unsigned short tmp_index;
        tmp_index = _write_index;
        _write_index = _read_index;
        _read_index = tmp_index;
        _lock.unlock();
    }
}

void CameraBase::get_img(Mat &dst) {
    _lock.lock();
    _buffer.at(_read_index).copyTo(dst);
    _lock.unlock();
}
