#include "utils.h"
#include "cv_config.h"
#include "camera.h"
#include <iostream>
#include <string>

int main(void) {
    std::cout << "Start" << std::endl;
    high_resolution_timer timer;
    OV5693Cam tx2_cam;
    int code = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter save_stream("cam_record.avi", code, 120, Size(IMAGE_WIDTH, IMAGE_HEIGHT));
    timer.start();
    int cur_focus = 0;
    Mat cur_frame;
    while (timer.get_duration() < 11) {
        cur_focus = timer.get_duration();
        cur_focus = cur_focus * 100;
        timer.cp();
        tx2_cam.set_focus(cur_focus);
        tx2_cam.get_img(cur_frame);
        cv::putText(cur_frame, std::to_string(cur_focus), cv::Point(100, 100), FONT_HERSHEY_SIMPLEX, 0.9,
                Scalar(0, 150, 100), 2, LINE_AA);
        save_stream << cur_frame;
    }
    save_stream.release();
    std::cout << "done" << std::endl;
}
