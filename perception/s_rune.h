#ifndef _RUNE_
#define _RUNE_

#include <caffe/caffe.hpp>
#include <caffe/util/io.hpp>
#include <caffe/blob.hpp>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <cassert>
#include <chrono>
#include <ratio>
#include <ctime>
#include "cv_config.h"
#include "camera.h"

using namespace cv;
using namespace std;

#define W_CTR_LOW       64
#define W_CTR_HIGH      192
#define H_CTR_LOW       32
#define H_CTR_HIGH      96
#define HW_MIN_RATIO    0.3
#define HW_MAX_RATIO    0.75

#define BATCH_SIZE      15
#define CROP_SIZE       32
#define DIGIT_SIZE      28

#define DISTILL_RED_TH  70
#define MIN_RED_DIG_AREA 50

class s_rune {
public:
    s_rune(string net_file = "./model/lenet.prototxt",
            string param_file = "./model/lenet.caffemodel");
    ~s_rune();

    /**
     * @brief get hit box (domain: 1-9). The function is blocking
     *        and will return after RUNE_DETECT_TIME_SPAN seconds.
     * @param cam ptr to our camera
     * @return position to hit
     */
    int get_hit_pos(CameraBase * cam);

    /**
     * @brief this function should be called before calling any other processing function.
     *         function call updates the current frame to the latest image captured from
     *         the camera.
     * @param CameraBase type pointer that has been properly instantiated
     * @return none
     */
    void update(CameraBase *cam);

    /**
     * @brief get angle to move to hit the desired digit.
     * @param cam ptr to camera object
     * @return data pair of angles. <pitch, yaw>. Angles are relative
     *         to current angle. Yaw angle is positive if we want to
     *         move to the right. Pitch angle is positive if we want
     *         to look upward.
     */
    pair<float, float> get_hit_angle(CameraBase * cam);

    pair<float, float> fire_get_hit_angle(CameraBase *cam);

    vector<Mat> fire_get_res(CameraBase *cam);

private:
    int x_min;
    int x_max;
    int y_min;
    int y_max;
    int cur_round_counter;
    float angle_d_yaw;
    float angle_d_pitch;

    Mat white_bin;
    Mat red_bin;
    Mat raw_img;
    Mat gray_img;
    Mat debug_img;
    Mat distilled_img;

    caffe::Net<float>       *net;
    caffe::Blob<float>      *input_layer;
    caffe::Blob<float>      *output_layer;
    vector<Mat>             w_digits;   // white digits in 28x28 gray scale
    vector<Mat>             r_digits;   // red digits in 28x28 gray scale
    vector<vector<Point> >  w_contours; // white contours
    vector<vector<Point> >  r_contours; // red contours
    vector<pair<vector<Point>, int> > loc_idx;  // Points are upper left corner (of contours)
    int cur_red_digits[5];
    int cur_white_digits[9];
    int new_white_seq[9];
    int new_red_seq[5];

    Point2f dst_points[4] = {Point2f(0, 0), Point2f(CROP_SIZE, 0),
                        Point2f(0, CROP_SIZE), Point2f(CROP_SIZE, CROP_SIZE)};

                        /* Small Rune Utility Functions*/
    void white_binarize();
    void contour_detect();
    void batch_generate();
    void digit_recog();
    bool distill_red_dig();
    bool red_contour_detect();
    void get_current_rune(CameraBase * cam);
    int calc_position_to_hit();
    Mat pad_digit(const Mat & src_img);
    void red_batch_generate();
    Mat red_digit_process(const Mat & src_img);

                        /* Fire Rune Utility Functions */

                        /* Common CV Utility Functions */
    void red_binarize();

    /**
     * @brief calculates the white digit sequence according to the current frame
     *        sequence satisfies the following spatial relationship
     *
     *              1   2   3
     *
     *              4   5   6
     *
     *              7   8   9
     *
     * @param an array of integer to be filled with the sequence
     * @return flag indicating whether successfully identified exact 9 digits or not
     */
    bool get_white_seq(vector<int> &seq);

    /**
     * @brief calculates the red digit sequence according to the current frame
     *        sequence satisfies the following spatial relationship
     *
     *              1   2   3   4   5
     *
     * @param an array of integer to be filled with the sequence
     * @return flag indicating whether successfully indentified exact 5 digits or not
     */
    bool get_red_seq(vector<int> &seq);

    /**
     * @brief feed in data batch to caffe model and utilize irrelevent class
     *        to filter out incorrect bounding boxes
     * @param predictions array of paired data in (idx, digit_id) format where
     *                    idx is the corresponding index within the w_contours
     *                    array, and digit_id is the predicted digit number
     */
    void network_inference(vector<pair<int, int> > &predictions,
            vector<Mat> & desired_digits, vector<vector<Point> > & desired_contours);

    bool fire_filter_contour(const vector<Point> & single_contour, const vector<Mat> & bgr);

    vector<vector<Point> > fire_get_contours(void);
};

#endif
