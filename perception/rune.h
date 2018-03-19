#ifndef _RUNE_
#define _RUNE_

#include <caffe/caffe.hpp>
#include <caffe/util/io.hpp>
#include <caffe/blob.hpp>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <vector>
#include <cstring>
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


class Rune {
public:
    Rune(string net_file = "./model/lenet.prototxt",
            string param_file = "./model/lenet.caffemodel");
    ~Rune();

    /**
     * @brief this function should be called before calling any other processing function.
     *         function call updates the current frame to the latest image captured from
     *         the camera.
     * @param CameraBase type pointer that has been properly instantiated
     * @return None
     */
    void update(CameraBase *cam);

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

private:
    Mat white_bin;
    Mat red_bin;
    Mat raw_img;
    Mat gray_img;
    Mat debug_img;

    caffe::Net<float>       *net;
    caffe::Blob<float>      *input_layer;
    caffe::Blob<float>      *output_layer;
    vector<Mat>             w_digits;   // white digits in 28x28 gray scale
    vector<Mat>             r_digits;   // red digits in 28x28 gray scale
    vector<vector<Point> >  w_contours; // white contours
    vector<vector<Point> >  r_contours; // red contours
    
    Point2f dst_points[4] = {Point2f(0, 0), Point2f(CROP_SIZE, 0),
                        Point2f(0, CROP_SIZE), Point2f(CROP_SIZE, CROP_SIZE)};

    void white_binarize();
    void red_binarize();
    void contour_detect();
    void batch_generate();
    void digit_recog();

    /**
     * @brief feed in data batch to caffe model and utilize irrelevent class
     *        to filter out incorrect bounding boxes
     * @param predictions array of paired data in (idx, digit_id) format where
     *                    idx is the corresponding index within the w_contours
     *                    array, and digit_id is the predicted digit number
     *                    for that specific contour location.
     * @return None
     */
    void network_inference(vector<pair<int, int> > &predictions);
};

#endif
