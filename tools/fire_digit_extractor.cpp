#include "s_rune.h"
#include "video_feeder.h"
#include <string>

int main(void) {
    int counter = 0;
    int frame_counter = 0;
    string file_path = "fire.mp4";
    CameraBase * cam = new VideoFeed(file_path);
    s_rune rune_getter;
    while (cam->is_alive()) {
        ++frame_counter;
        vector<Mat> detected_digit = rune_getter.fire_get_res(cam);
        if (frame_counter % 8 != 0) continue;
        for (const Mat & dgt : detected_digit) {
            //cv::imwrite(std::to_string(++counter) + ".jpg", dgt);
        }
    }
    return 0;
}
