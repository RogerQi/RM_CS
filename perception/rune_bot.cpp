#include "rune_bot.h"
#include <iostream>

Point2f dst_points[4] = {Point2f(0, 0), Point2f(CROP_SIZE, 0),
                    Point2f(0, CROP_SIZE), Point2f(CROP_SIZE, CROP_SIZE)};

bool cmp_x(const cv::Point & i, const cv::Point & j) { return i.x < j.x; }

bool cmp_y(const cv::Point & i, const cv::Point & j) { return i.y < j.y; }

namespace perception {

rune_bot::rune_bot(CameraBase *_cam) {
    my_cam = _cam;
    // init NN
    net = LeNet::create(RUNE_PROTOTXT_PATH, RUNE_MODEL_PATH);
}

rune_bot::~rune_bot(void) {
    // delete NN
    delete net;
}

pair<float, float> rune_bot::comm_get_abs_gimbal_angle(void) {
    pair<float, float> ret;
    this->cam_update();
    return ret;
}

// get current bounding boxes
bool rune_bot::comm_rune_prep(void) {
    this->cam_update();
    this->red_digits.clear();
    this->large_digits.clear();
    int cur_remaining_match_minute = 1; //@TODO: use data from judge system
    if (cur_remaining_match_minute < 3) {
        // fire rune
        this->large_digits = this->fire_acquire_digits();
    } else {
        // white rune
        this->large_digits = this->white_acquire_digits();
    }
    if (this->large_digits.size() != 9)
        return false;
    cv::Rect red_digits_rect = this->infer_red_digits_rect_from_large_digits();
    this->red_digits = this->red_acquire_digits(red_digits_rect);
    if (this->red_digits.size() != 5)
        return false;
    return true;
}

void rune_bot::cam_update(void) {
    //cam->get_img(raw_img);
    Mat cache_img = my_cam->cam_read();
    this->cur_frame.raw_img.upload(cache_img);
    if (cur_frame.raw_img.size() != Size(IMAGE_WIDTH, IMAGE_HEIGHT))
        cv::cuda::resize(cur_frame.raw_img, cur_frame.raw_img, Size(IMAGE_WIDTH, IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR);
    cv::cuda::cvtColor(cur_frame.raw_img, cur_frame.gray_img, cv::COLOR_BGR2GRAY);
    Mat temp;
    this->cur_frame.gray_img.download(temp);
    cv::threshold(temp, temp, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
    cur_frame.otsu_threshed_img.upload(temp);
    this->cur_frame.bgr.clear();
    cv::cuda::split(cur_frame.raw_img, cur_frame.bgr);
}

vector<digit_t> rune_bot::fire_acquire_digits(void) {
    vector<digit_t> ret;
    vector<vector<Point> > fire_contours = this->fire_get_contours();
    if (fire_contours.size() < 9)
        return ret; // halt!
    vector<cv::cuda::GpuMat> digit_images = this->batch_generate(fire_contours, this->cur_frame.gray_img, false);
    vector<predicted_class> nn_results = this->nn_inference(digit_images);
    for (int i = 0; i < nn_results.size(); ++i) {
        if (nn_results[i] == 0 || nn_results[i] == 10)
            continue;
        digit_t digit_instance;
        digit_instance.recent_results.push_back(nn_results[i]);
        digit_instance.contour = fire_contours[i];
        ret.push_back(digit_instance);
    }
    return ret;
}

vector<digit_t> rune_bot::white_acquire_digits(void) {
    vector<digit_t> ret;
    vector<vector<Point> > white_contours = this->white_get_contours();
    if (white_contours.size() < 9)
        return ret;
    vector<cv::cuda::GpuMat> digit_images = this->batch_generate(white_contours, this->cur_frame.gray_img, false);
    vector<predicted_class> nn_results = this->nn_inference(digit_images);
    for (int i = 0; i < nn_results.size(); ++i) {
        if (nn_results[i] == 0 || nn_results[i] == 10)
            continue;
        digit_t digit_instance;
        digit_instance.recent_results.push_back(nn_results[i]);
        digit_instance.contour = white_contours[i];
        ret.push_back(digit_instance);
    }
    return ret;
}

vector<digit_t> rune_bot::red_acquire_digits(const cv::Rect & possible_area) {
    vector<digit_t> ret;
    cv::cuda::GpuMat red_distilled;
    distill_color_channel(this->cur_frame.bgr, 2, red_distilled);
    cv::Rect red_display = this->infer_red_digits_rect_from_large_digits();
    vector<vector<Point> > red_contours = this->red_get_contours(red_display);
    if (red_contours.size() < 5)
        return ret;
    vector<cv::cuda::GpuMat> digit_images = this->batch_generate(red_contours, red_distilled, true);
    vector<predicted_class> nn_results = this->nn_inference(digit_images);
    for (int i = 0; i < nn_results.size(); ++i) {
        if (nn_results[i] == 0 || nn_results[i] == 10)
            continue;
        digit_t digit_instance;
        digit_instance.recent_results.push_back(nn_results[i]);
        digit_instance.contour = red_contours[i];
        ret.push_back(digit_instance);
    }
    return ret;
}

/* Helper Functions Start */

vector<cv::cuda::GpuMat> rune_bot::batch_generate(vector<vector<Point> > & contours,
    const cv::cuda::GpuMat & ori_gray_img, bool padding) {

    vector<cv::cuda::GpuMat> ret;

    int offset = (CROP_SIZE - DIGIT_SIZE) / 2;
    Point2f cnt_points[4];

    for (vector<Point> &cnt: contours) {
        if (padding) {
            int x_min = IMAGE_WIDTH + 1;
            int y_min = IMAGE_HEIGHT + 1;
            int x_max = -1;
            int y_max = -1;
            for (const cv::Point & pt : cnt) {
                if (pt.x < x_min) {
                    x_min = pt.x;
                } else if (pt.x > x_max) {
                    x_max = pt.x;
                }
                if (pt.y < y_min) {
                    y_min = pt.y;
                } else if (pt.y > y_max) {
                    y_max = pt.y;
                }
            }
            cv::Rect roi(x_min, y_min, x_max - x_min, y_max - y_min);
            Mat mask = cv::Mat::zeros(ori_gray_img.rows, ori_gray_img.cols, ori_gray_img.type());
            vector<vector<Point> > contour_wrap = {cnt};
            cv::drawContours(mask, contour_wrap, -1, 255, -1);
            cv::cuda::GpuMat cache_masked, gpu_mask;
            gpu_mask.upload(mask);
            ori_gray_img.copyTo(cache_masked, gpu_mask);
            cv::cuda::GpuMat digit_img(cache_masked, roi);
            ret.push_back(digit_img);
        } else {
            cv::cuda::GpuMat digit_img;
            sort(cnt.begin(), cnt.end(), cmp_y);
            if (cnt[0].x > cnt[1].x)
                swap(cnt[0], cnt[1]);
            if (cnt[2].x > cnt[3].x)
                swap(cnt[2], cnt[3]);
            for (size_t i = 0; i < 4; i++)
                cnt_points[i] = Point2f(cnt[i]);
            Mat M = cv::getPerspectiveTransform(cnt_points, dst_points);
            cv::cuda::warpPerspective(ori_gray_img, digit_img, M, Size(CROP_SIZE, CROP_SIZE));
            // No idea why cv::cuda::bitwise_not is not working. Use CPU implementation here...
            cv::Mat ret_digit_img;
            digit_img(cv::Rect(offset, offset, DIGIT_SIZE, DIGIT_SIZE)).download(ret_digit_img);
            cv::bitwise_not(ret_digit_img, ret_digit_img);
            digit_img.upload(ret_digit_img);
            ret.push_back(digit_img);
        }
    }
    return ret;
}

vector<predicted_class> rune_bot::nn_inference(std::vector<cv::cuda::GpuMat> & query) {
    vector<predicted_class> ret;
    if (query.size() == 0) return ret;
    std::vector<uint8_t> rs;
    std::vector<float>  conf;

    if (query.size() <= 32) {
        // base case
        this->net->predict(query, &rs, &conf);
        for (size_t i = 0; i < rs.size(); ++i) {
            ret.push_back(static_cast<predicted_class>(rs[i]));
        }
    } else {
        vector<predicted_class> subret;
        std::vector<cv::cuda::GpuMat> subquery(&query[32], &query[query.size() - 1]);
        query.erase(query.begin() + 32, query.end());
        this->net->predict(query, &rs, &conf);
        for (size_t i = 0; i < rs.size(); ++i) {
            ret.push_back(static_cast<predicted_class>(rs[i]));
        }
        subret = this->nn_inference(subquery);
        for (size_t i = 0; i < subret.size(); ++i) {
            ret.push_back(subret[i]);
        }
    }
    return ret;
}

vector<vector<Point> > rune_bot::red_get_contours(const cv::Rect & possible_area) {
    cv::cuda::GpuMat distilled_red_img;
    vector<Vec4i> hierarchy;
    vector<vector<Point> > contours, post_contours;

    // preprocess image
    cv::Mat red_mask = cv::Mat::zeros(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8UC1);
    red_mask(possible_area) = 1;
    distill_color_channel(this->cur_frame.bgr, 2, distilled_red_img, red_mask);
    cv::cuda::threshold(distilled_red_img, distilled_red_img, 70, 255, cv::THRESH_BINARY);
    Ptr<cuda::Filter> dilate_filter = cv::cuda::createMorphologyFilter(cv::MORPH_DILATE, CV_8UC1, cv::Mat::ones(5, 3, CV_8UC1));
    dilate_filter->apply(distilled_red_img, distilled_red_img);
    // find and format contours
    Mat cache_mat;
    distilled_red_img.download(cache_mat);
    cv::findContours(cache_mat, contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    for (const vector<Point> & ctr : contours) {
        cv::RotatedRect ctr_rect = cv::minAreaRect(ctr);
        if (ctr_rect.size.area() > 50) {
            cv::Point2f pts[4];
            ctr_rect.points(pts);
            std::vector<Point> temp;
            for(const Point2f & pt : pts)
                temp.push_back(pt);
            post_contours.push_back(temp);
        }
    }
    return post_contours;
}

vector<vector<Point> > rune_bot::white_get_contours(void) {
    vector<Vec4i> hierarchy;
    vector<vector<Point> > contours, post_contours;
    cv::Mat white_bin;
    cv::cuda::GpuMat white_bin_gpu_cache;
    Ptr<cuda::Filter> close_filter = cv::cuda::createMorphologyFilter(cv::MORPH_CLOSE, CV_8UC1, cv::Mat::ones(4, 8, CV_8UC1));
    close_filter->apply(this->cur_frame.otsu_threshed_img, white_bin_gpu_cache);

    white_bin_gpu_cache.download(white_bin);
    cv::findContours(white_bin, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    for (const vector<Point> & cnt: contours) {
        float width = (*std::max_element(cnt.begin(), cnt.end(), cmp_x)).x -
            (*std::min_element(cnt.begin(), cnt.end(), cmp_x)).x;
        float height = (*std::max_element(cnt.begin(), cnt.end(), cmp_y)).y -
            (*std::min_element(cnt.begin(), cnt.end(), cmp_y)).y;
        if (width >= W_CTR_LOW && width <= W_CTR_HIGH &&
                height >= H_CTR_LOW && height <= H_CTR_HIGH &&
                height / width >= HW_MIN_RATIO && height / width <= HW_MAX_RATIO) {
            vector<Point> approx;
            cv::approxPolyDP(cnt, approx, 0.05 * cv::arcLength(cnt, true), true);
            if (approx.size() == 4 && cv::isContourConvex(approx))
                post_contours.push_back(approx);
        }
    }
    return post_contours;
}

vector<vector<Point> > rune_bot::fire_get_contours(void) {
    vector<Vec4i> hierarchy;
    vector<vector<Point> > pre_contours, contours, post_contours;
    cv::Mat temp;
    this->cur_frame.otsu_threshed_img.download(temp);
    cv::findContours(temp, pre_contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    for (const vector<Point> & ctr : pre_contours) {
        double area_size = cv::contourArea(ctr);
        if (area_size > 400 && area_size < 4000)
            contours.push_back(ctr);
    }
    for (const vector<Point> & ctr : contours) {
        if (this->fire_filter_contour(ctr)) {
            // use bounding rect to get a rect contour and image
            cv::Rect bounding_box = cv::boundingRect(ctr);
            bounding_box.width *= 1.1;
            bounding_box.height *= 1.1;
            cv::Point top_left = bounding_box.tl();
            cv::Point top_right(top_left.x + bounding_box.width, top_left.y);
            cv::Point bot_left(top_left.x, top_left.y + bounding_box.height);
            cv::Point bot_right(bot_left.x + bounding_box.width, bot_left.y);
            vector<Point> bbox_ctr = {bot_left, bot_right, top_left, top_right};
            post_contours.push_back(bbox_ctr);
        }
    }
    return post_contours;
}

cv::Rect rune_bot::infer_red_digits_rect_from_large_digits(void) {
    int x_min = IMAGE_WIDTH + 1;
    int y_min = IMAGE_HEIGHT + 1;
    int x_max = -1;
    int y_max = -1;
    for (const digit_t & dgt : this->large_digits) {
        for (const cv::Point & pt : dgt.contour) {
            if (pt.x < x_min) {
                x_min = pt.x;
            } else if (pt.x > x_max) {
                x_max = pt.x;
            }
            if (pt.y < y_min) {
                y_min = pt.y;
            } else if (pt.y > y_max) {
                y_max = pt.y;
            }
        }
    }
    cv::Rect ret(x_min, y_min, x_max - x_min, y_max - y_min);
    return ret;
}

bool rune_bot::fire_filter_contour(const vector<Point> & single_contour) {
    vector<float> mean_thresh = {150, 210, 230};
    vector<float> std_thresh = {50, 40, 25};
    vector<vector<Point> > contour_wrap = {single_contour};
    Mat mask = cv::Mat::zeros(this->cur_frame.bgr[0].rows, this->cur_frame.bgr[0].cols, CV_8UC1);
    cv::drawContours(mask, contour_wrap, -1, 255, -1);
    vector<float> my_mean, my_std;
    for (const cv::cuda::GpuMat & my_mat : this->cur_frame.bgr) {
        cv::Scalar temp_mean, temp_std;
        Mat cache_channel;
        my_mat.download(cache_channel);
        cv::meanStdDev(cache_channel, temp_mean, temp_std, mask);
        my_mean.push_back(temp_mean.val[0]);
        my_std.push_back(temp_std.val[0]);
    }
    if (naive_thres_test(my_mean, mean_thresh, 55) && naive_thres_test(my_std, std_thresh, 20))
        return true;
    return false;
}

vector<digit_t> rune_bot::recognize_large_digits(void) {
    vector<vector<Point> > large_contours;
    for (size_t i = 0; i < this->large_digits.size(); ++i)
        large_contours.push_back(this->large_digits[i].contour);
    assert(large_contours.size() == 9);
    vector<cv::cuda::GpuMat> digit_images = this->batch_generate(large_contours, this->cur_frame.gray_img, false);
    vector<predicted_class> nn_results = this->nn_inference(digit_images);
    for (int i = 0; i < nn_results.size(); ++i) {
        if (nn_results[i] == 0 || nn_results[i] == 10)
            continue;
        digit_t digit_instance;
        digit_instance.recent_results.push_back(nn_results[i]);
        digit_instance.contour = fire_contours[i];
        ret.push_back(digit_instance);
    }
    return ret;
}

}
