#include "cv_config.h"
#include "camera.h"
#include "rune.h"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

Rune::Rune() {}

Rune::~Rune() {}

void Rune::update(CameraBase *cam) {
    cam->get_img(raw_img);
    if (raw_img.size() != Size(IMAGE_WIDTH, IMAGE_HEIGHT))
        cv::resize(raw_img, raw_img, Size(IMAGE_WIDTH, IMAGE_HEIGHT));
    if (DEBUG) {
        raw_img.copyTo(debug_img);
        imshow("raw image", raw_img);
        waitKey(1);
    }
}

void Rune::white_binarize() {
    cv::cvtColor(raw_img, white_bin, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(white_bin, white_bin, Size(3,3), 0);
    cv::threshold(white_bin, white_bin, 0, 255, cv::THRESH_BINARY+cv::THRESH_OTSU);
    cv::morphologyEx(white_bin, white_bin, cv::MORPH_CLOSE, Mat::ones(4, 8, CV_8UC1));
    if (DEBUG) {
        imshow("white digit binarized", white_bin);
        waitKey(1);
    }
}

bool compare_x(Point &i, Point &j) {
    return i.x < j.x;
}

bool compare_y(Point &i, Point &j) {
    return i.y < j.y;
}

void Rune::contour_detect() {
    vector<Vec4i>           hierarchy;
    vector<vector<Point> >  contours;

    w_contours.clear();

    cv::findContours(white_bin, contours, hierarchy,
            cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    for (auto cnt: contours) {
        float width = (*max_element(cnt.begin(), cnt.end(), compare_x)).x -
            (*min_element(cnt.begin(), cnt.end(), compare_x)).x;
        float height = (*max_element(cnt.begin(), cnt.end(), compare_y)).y -
            (*min_element(cnt.begin(), cnt.end(), compare_y)).y;
        if (width >= W_CTR_LOW && width <= W_CTR_HIGH && 
                height >= H_CTR_LOW && height <= H_CTR_HIGH &&
                height / width >= HW_MIN_RATIO && height / width <= HW_MAX_RATIO) {
            vector<Point> approx;
            cv::approxPolyDP(cnt, approx, 0.05 * cv::arcLength(cnt, true), true);
            if (approx.size() == 4 && cv::isContourConvex(approx))
                w_contours.push_back(approx);
        }
    }
    if (DEBUG) {
        cv::drawContours(debug_img, w_contours, -1, Scalar(0, 255, 0), 3);
        imshow("contour detection", debug_img);
        waitKey(1);
    }
}

bool Rune::get_white_seq(vector<int> &seq) {
    white_binarize(); 
    contour_detect();
    return true;
}

bool Rune::get_red_seq(vector<int> &seq) {
    return true;
}
