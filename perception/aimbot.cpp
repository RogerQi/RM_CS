#include "aimbot.h"

void distill_color(const Mat & src_img, Mat & dst_img, string color_type) {
    std::vector<cv::Mat> bgr;
    cv::split(src_img, bgr);
    if (color_type == "red") {
        cv::subtract(bgr[2], bgr[1], dst_img);
    } else {
      //assume it's blue
        cv::subtract(bgr[0], bgr[2], dst_img);
    }
}
