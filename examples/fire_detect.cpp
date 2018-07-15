#include "rune_bot.h"
#include "cv_config.h"
#include "video_feeder.h"
#include <string>

int main(void) {
    string file_path = "fire.mp4";
    CameraBase * cam = new VideoFeed(file_path);
    int code = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter save_stream("fire_rune_detect.avi", code, 30, Size(IMAGE_WIDTH, IMAGE_HEIGHT));
    perception::rune_bot rune_getter(cam);
    while (cam->is_alive()) {
        
    }
    /*
    rune_getter.cam_update();
    vector<digit_t> fire_digits = rune_getter.fire_acquire_digits();
    vector<digit_t> red_digits;
    bool detected_ = false;
    while (cam->is_alive()) {
        rune_getter.cam_update();
        vector<digit_t> fire_digits;
        if (detected_) {
            fire_digits = rune_getter.recognize_large_digits();
        } else {
            fire_digits = rune_getter.fire_acquire_digits();
            if(fire_digits.size() == 9) {
                red_digits = rune_getter.red_acquire_digits();
                for (size_t i = 0; i < red_digits.size(); ++i) fire_digits.push_back(red_digits[i]);
                detected_ = false;
            }
        }
        Mat cur_frame;
        rune_getter.get_cur_frame().download(cur_frame);
        vector<vector<Point> > ctrs;
        for (size_t i = 0; i < fire_digits.size(); ++i) {
            ctrs.push_back(fire_digits[i].contour);
            putText(cur_frame, to_string((*--fire_digits[i].recent_results.end())), fire_digits[i].contour[0],
                FONT_HERSHEY_SIMPLEX, 0.9, Scalar(0, 150, 100), 2, LINE_AA);
        }

        cv::drawContours(cur_frame, ctrs, -1, cv::Scalar(255, 0, 0), 3);
        save_stream << cur_frame;
    }
    */
    save_stream.release();
    return 0;
}
