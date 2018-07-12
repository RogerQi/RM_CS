#ifndef CV_UTILS_H_
#define CV_UTILS_H_

#include <opencv2/opencv.hpp>
#include <vector>

//template<class T>
//inline cv::Mat distill_color(const T & iter_bgr);

inline void distill_color_channel(const std::vector<cv::cuda::GpuMat> & bgr,
            int desired_channel, cv::cuda::GpuMat & dst_img, cv::InputArray _mask = cv::noArray()) {
    int base_color = desired_channel - 1;
    if (base_color < 0) base_color += 3;
    cv::cuda::subtract(bgr[desired_channel], bgr[base_color], dst_img, _mask);
}

template<class T, class V>
inline bool naive_thres_test(const vector<T> & query, const vector<V> & baseline, float thresh) {
    assert(query.size() == baseline.size());
    for (size_t i = 0; i < query.size(); ++i) {
        if (fabs(query[i] - baseline[i]) > thresh) return false;
    }
    return true;
}

#endif
