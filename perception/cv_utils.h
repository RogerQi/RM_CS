#ifndef CV_UTILS_H_
#define CV_UTILS_H_

#include <opencv2/opencv.hpp>
#include <vector>

//template<class T>
//inline cv::Mat distill_color(const T & iter_bgr);

inline void distill_color(cv::Mat bgr[3], int desired_channel, cv::Mat & dst_img) {
    cv::Mat ret;
    int base_color = desired_channel - 1;
    if (base_color < 0) base_color += 3;
    cv::split(bgr[desired_channel], bgr[base_color], dst_img);
}

#endif
