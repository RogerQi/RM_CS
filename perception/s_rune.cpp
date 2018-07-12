#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/util/io.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iostream>
#include "cv_utils.h"
#include "cv_config.h"
#include "camera.h"
#include "s_rune.h"

using namespace caffe;
using namespace std;
using namespace cv;
using namespace std::chrono;

bool cmp_x(Point &i, Point &j) { return i.x < j.x; }

bool cmp_y(Point &i, Point &j) { return i.y < j.y; }

bool cmp_px(pair<Point, int> &i, pair<Point, int> &j) { return i.first.x < j.first.x; }

bool cmp_cnt_px(pair<vector<Point>, int> &i, pair<vector<Point>, int> &j) { return i.first[0].x < j.first[0].x; }

bool cmp_py(pair<Point, int> &i, pair<Point, int> &j) { return i.first.y < j.first.y; }

bool cmp_cnt_py(pair<vector<Point>, int> &i, pair<vector<Point>, int> &j) { return i.first[0].y < j.first[0].y; }

template<class T>
int argmax(const T * data, size_t length) {
    int max_idx = 0;
    for (size_t i = 1; i < length; i++)
        if (data[i] > data[max_idx])
            max_idx = i;

    return max_idx;
}

bool array_array_equal(int int_arr[], int int_arr_2[], int length){
    for(int i = 0; i < length; i++){
        if(int_arr[i] != int_arr_2[i])
            return false;
    }
    return true;
}

s_rune::s_rune(string net_file, string param_file) {
#ifdef CPU_ONLY
    Caffe::set_mode(Caffe::CPU);
#else
    Caffe::DeviceQuery();
    Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(0);
#endif
    net = new Net<float>(net_file.c_str(), caffe::TEST);
    net->CopyTrainedLayersFrom(param_file.c_str());
    input_layer = net->input_blobs()[0];
    input_layer->Reshape(BATCH_SIZE, 1, DIGIT_SIZE, DIGIT_SIZE);
    net->Reshape();
    input_layer = net->input_blobs()[0];
    output_layer = net->output_blobs()[0];
    angle_d_pitch = (CV_CAMERA_FOV_Y) / (IMAGE_HEIGHT * 1.0);
    angle_d_yaw = (CV_CAMERA_FOV_X) / (IMAGE_WIDTH * 1.0);
}

s_rune::~s_rune() {

}

void s_rune::update(CameraBase *cam) {
    //cam->get_img(raw_img);
    raw_img = cam->cam_read();
    if (raw_img.size() != Size(IMAGE_WIDTH, IMAGE_HEIGHT))
        cv::resize(raw_img, raw_img, Size(IMAGE_WIDTH, IMAGE_HEIGHT), 0, 0, cv::INTER_LINEAR);
#ifdef DEBUG
    raw_img.copyTo(debug_img);
    imshow("raw image", raw_img);
    waitKey(1);
#endif
}

int s_rune::get_hit_pos(CameraBase * cam){
    get_current_rune(cam);
    return calc_position_to_hit();
}

pair<float, float> s_rune::get_hit_angle(CameraBase * cam){
    get_current_rune(cam);
    int pos = calc_position_to_hit();
    int cur_digit = cur_white_digits[pos];
    vector<Point> desired_cnt;
    for(size_t i = 0; i < loc_idx.size(); i++){
        if(loc_idx[i].second == cur_digit){
            desired_cnt = loc_idx[i].first;
            break;
        }
    }
    float x = 0;
    float y = 0;
    for(const Point & pt : desired_cnt){
        x += pt.x;
        y += pt.y;
    }
    x = x / static_cast<int>(desired_cnt.size());
    y = y / static_cast<int>(desired_cnt.size());
    float yaw = - (IMAGE_WIDTH / 2.0 - x) * angle_d_yaw;
    float pitch = (IMAGE_HEIGHT / 2.0 - y) * angle_d_pitch;
    return pair<float, float>(pitch, yaw);
}

int s_rune::calc_position_to_hit(void){
    if(array_array_equal(new_red_seq, cur_red_digits, 5))
        cur_round_counter += 1;
    else
        //new round; recognition error
        cur_round_counter = 0;

    if(cur_round_counter >= 5)
        //guess you are debugging; set it to new round
        cur_round_counter = 0;

    for(int i = 0; i < 9; i++)
        cur_white_digits[i] = new_white_seq[i];
    for(int i = 0; i < 5; i++)
        cur_red_digits[i] = new_red_seq[i];

    //assume current red digits and white digits are accurate
    int desired_number = cur_red_digits[cur_round_counter];
    for(int i = 0; i < 9; i++)
        if(cur_white_digits[i] == desired_number)
            return i + 1;

    std::cerr << "You should never get here!!!!!! 1-9 are not in white digits" << std::endl;
    return 1;
}

void s_rune::get_current_rune(CameraBase * cam) {
    high_resolution_clock::time_point start_time = high_resolution_clock::now();
    high_resolution_clock::time_point cur_time = high_resolution_clock::now();
    duration<double> time_elapsed = duration_cast<duration<double>>(cur_time - start_time);
    int red_matrix[5][9];
    int white_matrix[9][9];

    for(int i = 0; i < 5; i++)
        fill_n(red_matrix[i], 9, 0);
    for(int i = 0; i < 9; i++)
        fill_n(white_matrix[i], 9, 0);

    while(time_elapsed < duration<double>(RUNE_DETECT_TIME_SPAN)) {
        cur_time = high_resolution_clock::now();
        time_elapsed = duration_cast<duration<double> >(cur_time - start_time);
        vector<int> white_seq, red_seq;
        this->update(cam);
        bool success;
        success = this->get_white_seq(white_seq);
        if(!(success) || white_seq.size() < 9)
            continue;
        success = this->get_red_seq(red_seq);
        if(!(success) || red_seq.size() < 5)
            continue;
        for(size_t i = 0; i < 9; i++) {
            if(white_seq[i] == 0)
                continue; //there are no zeros!!
            ++white_matrix[i][white_seq[i] - 1];
        }
        for(size_t i = 0; i < 5; i++){
            if(red_seq[i] == 0)
                continue;
            ++red_matrix[i][red_seq[i] - 1];
        }
    }
    for(int i = 0; i < 5; i++)
        new_red_seq[i] = argmax(red_matrix[i], 9) + 1;
    for(int i = 0; i < 9; i++)
        new_white_seq[i] = argmax(white_matrix[i], 9) + 1;
}

void s_rune::white_binarize() {
    cv::cvtColor(raw_img, gray_img, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray_img, white_bin, Size(3,3), 0);
    cv::threshold(white_bin, white_bin, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
    cv::morphologyEx(white_bin, white_bin, cv::MORPH_CLOSE, Mat::ones(4, 8, CV_8UC1));
#ifdef DEBUG
    imshow("white digit binarized", white_bin);
    waitKey(1);
#endif
}

void s_rune::contour_detect() {
    vector<Vec4i>           hierarchy;
    vector<vector<Point> >  contours;

    w_contours.clear();

    cv::findContours(white_bin, contours, hierarchy,
            cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    for (auto &cnt: contours) {
        float width = (*max_element(cnt.begin(), cnt.end(), cmp_x)).x -
            (*min_element(cnt.begin(), cnt.end(), cmp_x)).x;
        float height = (*max_element(cnt.begin(), cnt.end(), cmp_y)).y -
            (*min_element(cnt.begin(), cnt.end(), cmp_y)).y;
        if (width >= W_CTR_LOW && width <= W_CTR_HIGH &&
                height >= H_CTR_LOW && height <= H_CTR_HIGH &&
                height / width >= HW_MIN_RATIO && height / width <= HW_MAX_RATIO) {
            vector<Point> approx;
            cv::approxPolyDP(cnt, approx, 0.05 * cv::arcLength(cnt, true), true);
            if (approx.size() == 4 && cv::isContourConvex(approx))
                w_contours.push_back(approx);
        }
    }
#ifdef DEBUG
    cv::drawContours(debug_img, w_contours, -1, Scalar(0, 255, 0), 3);
    imshow("contour detection", debug_img);
    waitKey(1);
#endif
}

vector<Mat> s_rune::fire_get_res(CameraBase *cam) {
    this->update(cam);
    vector<vector<Point> > filtered_contours = fire_get_contours();
    w_contours.clear();
    for (const vector<Point> & cnt : filtered_contours) {
        cv::Rect bounding_box = cv::boundingRect(cnt);
        bounding_box.width *= 1.1;
        bounding_box.height *= 1.1;
        cv::Point top_left = bounding_box.tl();
        cv::Point top_right(top_left.x + bounding_box.width, top_left.y);
        cv::Point bot_left(top_left.x, top_left.y + bounding_box.height);
        cv::Point bot_right(bot_left.x + bounding_box.width, bot_left.y);
        vector<Point> bbox_ctr = {bot_left, bot_right, top_left, top_right};
        w_contours.push_back(bbox_ctr);
    }
    batch_generate();
    vector<pair<int, int> > predictions;
    network_inference(predictions, w_digits);
    return w_digits;
}

vector<vector<Point> > s_rune::fire_get_contours(void) {
    Mat my_gray_binarized_img;
    vector<Vec4i> hierarchy;
    vector<vector<Point> > pre_contours, contours, post_contours;
    cv::cvtColor(raw_img, gray_img, cv::COLOR_BGR2GRAY);
    cv::threshold(gray_img, my_gray_binarized_img, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
    cv::findContours(my_gray_binarized_img, pre_contours, hierarchy, cv::RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    for (const vector<Point> & ctr : pre_contours) {
        double area_size = cv::contourArea(ctr);
        if (area_size > 400 && area_size < 4000)
            contours.push_back(ctr);
    }
    std::vector<Mat> bgr;
    cv::split(raw_img, bgr);
    for (const vector<Point> & ctr : contours) {
        if (fire_filter_contour(ctr, bgr))
            post_contours.push_back(ctr);
    }
    return post_contours;
}

bool s_rune::fire_filter_contour(const vector<Point> & single_contour, const vector<Mat> & bgr) {
    vector<float> mean_thresh = {150, 210, 230};
    vector<float> std_thresh = {50, 40, 25};
    vector<float> random_std_thresh = {10, 10, 8};
    vector<vector<Point> > contour_wrap = {single_contour};
    Mat mask = cv::Mat::zeros(bgr[0].rows, bgr[0].cols, CV_8UC1);
    cv::drawContours(mask, contour_wrap, -1, 255, -1);
    vector<float> my_mean, my_std;
    for (const Mat & my_mat : bgr) {
        cv::Scalar temp_mean, temp_std;
        cv::meanStdDev(my_mat, temp_mean, temp_std, mask);
        my_mean.push_back(temp_mean.val[0]);
        my_std.push_back(temp_std.val[0]);
    }
    if (naive_thres_test(my_mean, mean_thresh, 55) && naive_thres_test(my_std, std_thresh, 20))
        return true;
    return false;
}

void s_rune::batch_generate() {
    int offset = (CROP_SIZE - DIGIT_SIZE) / 2;
    Mat M;
    Point2f cnt_points[4];
    w_digits.clear();
    int i = 0;

    for (vector<Point> &cnt: w_contours) {
        sort(cnt.begin(), cnt.end(), cmp_y);
        if (cnt[0].x > cnt[1].x)
            swap(cnt[0], cnt[1]);
        if (cnt[2].x > cnt[3].x)
            swap(cnt[2], cnt[3]);
        for (size_t i = 0; i < 4; i++)
            cnt_points[i] = Point2f(cnt[i]);
        Mat digit_img;
        M = cv::getPerspectiveTransform(cnt_points, dst_points);
        cv::warpPerspective(gray_img, digit_img, M, Size(CROP_SIZE, CROP_SIZE));
        cv::bitwise_not(digit_img(cv::Rect(offset, offset, DIGIT_SIZE, DIGIT_SIZE)), digit_img);
        w_digits.push_back(digit_img);
    }
}

void s_rune::network_inference(vector<pair<int, int> > &predictions, vector<Mat> & desired_digits){
    float *input_data = input_layer->mutable_cpu_data();
    float *output_data = output_layer->mutable_cpu_data();
    for (auto &dig: desired_digits) {
        Mat channel(DIGIT_SIZE, DIGIT_SIZE, CV_32FC1, input_data);
        dig.convertTo(channel, CV_32FC1);
        channel /= 255;
        input_data += DIGIT_SIZE * DIGIT_SIZE;
    }

    net->Forward();

    for (size_t i = 0; i < desired_digits.size(); i++) {
        int dig_id = argmax(output_data, output_layer->channels());
        output_data += output_layer->channels();
        if (dig_id != 10)    // irrelevent class number
            predictions.push_back(pair<int, int>(i, dig_id));
    }
#ifdef DEBUG
    for(auto &p: predictions) {
        int dig_id = p.second;
        Point loc = desired_contours[p.first][0];
        loc.y -= 20;
        putText(debug_img, to_string(dig_id), loc, FONT_HERSHEY_SIMPLEX, 0.9,
                Scalar(0, 150, 100), 2, LINE_AA);
    }
    if (predictions.size() == 9)
        imshow("white digit recognition", debug_img);
    else if (predictions.size() == 5)
        imshow("red digit recognition", debug_img);
    waitKey(1);
#endif
}

bool s_rune::get_white_seq(vector<int> &seq) {
    white_binarize();
    contour_detect();
    if (w_contours.size() > BATCH_SIZE || w_contours.size() < 9)
        return false;
    batch_generate();

    vector<pair<int, int> > predictions;
    network_inference(predictions, w_digits);
    if (predictions.size() != 9)
        return false;

    loc_idx.clear();
    for (size_t i = 0; i < predictions.size(); i++)
        loc_idx.push_back(pair<vector<Point>, int>(w_contours[predictions[i].first],
                    predictions[i].second));
    sort(loc_idx.begin(), loc_idx.end(), cmp_cnt_py);
    sort(loc_idx.begin(), loc_idx.begin()+3, cmp_cnt_px);
    sort(loc_idx.begin()+3, loc_idx.begin()+6, cmp_cnt_px);
    sort(loc_idx.begin()+6, loc_idx.end(), cmp_cnt_px);

    x_min = max(loc_idx[1].first[0].x - 20, 0);
    x_max = min(loc_idx[2].first[0].x + 20, IMAGE_WIDTH);
    y_min = max(max(loc_idx[0].first[0].y, loc_idx[2].first[0].y) - 20, 0);

    for (auto &li: loc_idx)
        seq.push_back(li.second);
    return true;
}

bool s_rune::get_red_seq(vector<int> &seq) {
    if (!distill_red_dig() || !red_contour_detect())
        return false;
    red_batch_generate();
    vector<pair<int, int> > predictions;
    network_inference(predictions, r_digits);
    if(predictions.size() < 5){
        return false;
    }

    vector<pair<Point, int> > r_loc_idx;
    for (size_t i = 0; i < predictions.size(); i++)
        r_loc_idx.push_back(pair<Point, int>(r_contours[predictions[i].first][0],
                    predictions[i].second));
    sort(r_loc_idx.begin(), r_loc_idx.end(), cmp_px);
    for(const pair<Point, int> & pa : r_loc_idx){
        seq.push_back(pa.second);
    }
    return true;
}

bool s_rune::red_contour_detect(void){
    vector<Vec4i> hierarchy;
    vector<vector<Point> > temp_contour;
    r_contours.clear();
    findContours(distilled_img, temp_contour, hierarchy, cv::RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    if(temp_contour.size() < 5)
        return false;

    for(auto it = temp_contour.begin(); it != temp_contour.end(); ++it) {
        vector<Point> cnt = *(it);
        RotatedRect rect = minAreaRect(cnt);
        if(rect.size.area() < MIN_RED_DIG_AREA)
            continue;
        else {
            Point2f pts[4];
            rect.points(pts);
            vector<Point> temp;
            for(const Point2f & pt : pts)
                temp.push_back(pt);
            r_contours.push_back(temp);
        }
    }
    if(r_contours.size() < 5)
        return false;
#ifdef DEBUG
    cv::drawContours(debug_img, r_contours, -1, Scalar(0, 255, 0), 3);
    imshow("red detection", debug_img);
    waitKey(1);
#endif
    return true;
}

Mat s_rune::pad_digit(const Mat & src_img){
    assert(src_img.cols <= DIGIT_SIZE && src_img.rows <= DIGIT_SIZE);
    Mat painting = Mat::zeros(DIGIT_SIZE, DIGIT_SIZE, CV_8UC1);
    int x_offset = (DIGIT_SIZE - src_img.cols) / 2;
    int y_offset = (DIGIT_SIZE - src_img.rows) / 2;
    src_img.copyTo(painting(Rect(x_offset, y_offset, src_img.cols, src_img.rows)));
    return painting;
}

void s_rune::red_batch_generate(){
    int offset = (CROP_SIZE - DIGIT_SIZE) / 2;
    Mat M;
    Point2f cnt_points[4];
    r_digits.clear();
    int i = 0;

    for (vector<Point> &cnt: r_contours) {
        sort(cnt.begin(), cnt.end(), cmp_y);
        if (cnt[0].x > cnt[1].x)
            swap(cnt[0], cnt[1]);
        if (cnt[2].x > cnt[3].x)
            swap(cnt[2], cnt[3]);
        for (size_t i = 0; i < 4; i++)
            cnt_points[i] = Point2f(cnt[i]);
        Mat digit_img;
        Point w_diff = cnt[0] - cnt[1];
        Point h_diff = cnt[0] - cnt[2];
        int w = cv::sqrt(w_diff.x*w_diff.x + w_diff.y*w_diff.y);
        int h = cv::sqrt(h_diff.x*h_diff.x + h_diff.y*h_diff.y);

        Point2f red_dst_points[4] = {Point2f(0, 0), Point2f(w, 0),
                            Point2f(0, h), Point2f(w, h)};
        M = cv::getPerspectiveTransform(cnt_points, red_dst_points);
        cv::warpPerspective(distilled_img, digit_img, M, Size(w, h));
        double scale = (DIGIT_SIZE - 4) * 1.0 / max(w, h);
        cv::resize(digit_img, digit_img, Size(max(w*scale, 1.0), max(h*scale, 1.0)));
        digit_img = pad_digit(digit_img);
        r_digits.push_back(digit_img);
#ifdef DEBUG
        std::ostringstream ss;
        ss << "digit " << i;
        imshow(ss.str().c_str(), digit_img);
        waitKey(1);
        i++;
#endif
    }
}

bool s_rune::distill_red_dig(void){
    std::vector<Mat> bgr;
    cv::split(raw_img, bgr);
    cv::subtract(bgr[2], bgr[1], distilled_img);
    distilled_img = distilled_img(cv::Rect(x_min, 0,
                min(distilled_img.cols, x_max)-x_min, y_min));
    if (!distilled_img.cols || !distilled_img.rows)
        return false;
    cv::threshold(distilled_img, distilled_img, DISTILL_RED_TH, 255, THRESH_BINARY);
    cv::dilate(distilled_img, distilled_img, Mat::ones(5, 3, CV_8UC1));
#ifdef DEBUG
    raw_img(cv::Rect(x_min, 0,
                min(raw_img.cols, x_max)-x_min, y_min)).copyTo(debug_img);
    cv::imshow("distilled image", distilled_img);
    cv::imshow("cropped red digit", debug_img);
    cv::waitKey(1);
#endif
    return true;
}
