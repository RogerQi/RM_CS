#include "camera_saver.h"

video_saver::video_saver(CameraBase *_cam, unsigned int interval_ms) {
    this->my_cam = _cam;
    this->sleep_ms = interval_ms;
    this->start();
}

video_saver::~video_saver() {
    // nothing
}

void video_saver::start(void) {
    std::thread t(&video_saver::interval_saver, this, this->sleep_ms);
    t.detach();
}

void video_saver::interval_saver(unsigned int sleep_sec) {
    while (true) {
        this->save_image();
        if (sleep_sec)
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_sec));
    }
}

void video_saver::save_image(void) {
    cv::Mat cur_frame;
    this->my_cam->get_img(cur_frame);
    cv::imwrite(this->save_path_prefix + std::to_string(++this->save_counter) + ".png", cur_frame);
}
