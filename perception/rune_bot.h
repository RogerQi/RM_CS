#ifndef RUNE_BOT_H_
#define RUNE_BOT_H_

#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <cassert>
#include <chrono>
#include <ratio>
#include <ctime>
#include "camera.h"
#include "LeNet.h"
#include "cv_utils.h"
#include "cv_config.h"
#include "rune_config.h"

typedef int idx;
typedef int predicted_class;

using std::vector;
using std::pair;
using cv::Point;
using cv::Mat;

typedef struct {
    cv::cuda::GpuMat raw_img;
    cv::cuda::GpuMat gray_img;
    cv::cuda::GpuMat otsu_threshed_img;
    std::vector<cv::cuda::GpuMat> bgr;
} rune_detect_frame_t;

typedef struct {
    vector<Point> contour;
    vector<int> recent_results;
} digit_t;

namespace perception {
    class rune_bot {
    public:
        rune_bot(CameraBase *_cam);

        ~rune_bot(void);

        pair<float, float> comm_get_abs_gimbal_angle(void);

        bool comm_rune_prep(void);
    private:
        CameraBase *my_cam;

        vector<digit_t> large_digits, red_digits;

        rune_detect_frame_t cur_frame;

        LeNet *net;

        void cam_update(void);

        vector<digit_t> fire_acquire_digits(void);

        vector<digit_t> white_acquire_digits(void);

        vector<digit_t> red_acquire_digits(const cv::Rect & possible_area);

    protected:

        vector<cv::cuda::GpuMat> batch_generate(vector<vector<Point> > & contours,
                                        const cv::cuda::GpuMat & ori_gray_img, bool padding);

        vector<predicted_class> nn_inference(std::vector<cv::cuda::GpuMat> & query);

        vector<vector<Point> > red_get_contours(const cv::Rect & possible_area);

        vector<vector<Point> > white_get_contours(void);

        vector<vector<Point> > fire_get_contours(void);

        cv::Rect infer_red_digits_rect_from_large_digits(void);

        bool fire_filter_contour(const vector<Point> & single_contour);
    };
}

#endif
