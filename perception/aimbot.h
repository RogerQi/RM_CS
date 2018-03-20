#ifndef AIMBOT_H_
#define AIMBOT_H_

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include "camera.h"
#include <algorithm>
#include <math.h>
#include "cv_config.h"
#include "aimbot_config.h"

using std::string;
using namespace cv;

struct armor_loc{
    float center_x;
    float center_y;
    float width;
    float height;
    float ang;
};

    /* Common Functions */

/*
 * @brief Simple template function that return the largest variable of two variables of same type
 * @param a variable of type T
 * @param b another variable of type T
 * @return largest of the two
 */
template<class T> const T& max_of_two(const T& a, const T&b) {return (a>b)? a:b;}

/*
 * @brief translate armor loc into cv::RotatedRect; should be deprecated.
 * @param al armor_loc object
 * @return cv::RotatedRect
 */
RotatedRect armor_loc_2_rotated_rect(armor_loc al);

/**
 * Magic function that highlights red or blue area
 * @param src_img ref to src_img
 * @param dst_img ref to an empty img
 * @param color_type string. It should be either "blue" or "red"
 */
void distill_color(const Mat & src_img, Mat & dst_img, string color_type);

/**
 * @brief helper function to draw cv::RotatedRect on Mat
 * @param mat_to_draw image to be modified
 * @param rect_to_draw rotated rectangle to be drawn
 */
void draw_rotated_rect(Mat & mat_to_draw, RotatedRect rect_to_draw);

/**
 * @brief helper function to calculate aspect ratio (y offset is considered height)
 * @param light a rotatedrect object (usually a light bar)
 * @return aspect ratio
 */
float _cal_aspect_ratio(RotatedRect light);

/**
 * @brief get POI with respect to original image size (for cropping)
 * @param crop_distilled preprocessed image
 */
Point2f _get_point_of_interest(const Mat & crop_distilled);

/**
 * @brief dynamically crop image
 */
Mat _image_cropper(const Mat & frame, Point2f poi);

/*
 * An abstract base class for assisted aiming
 */
class aimbot{
public:
    /**
     * constructor for generic aimbot class; initialize configuration into memory
     */
    aimbot() {
        //do nothing
    }

    /**
     * destructor for generic aimbot class;
     */
    ~aimbot(){
        //do nothing
    }

    /**
     * Process image (frame) in current video buffer; pure virtual function to be implemented
     * @return vector of cv::Rect object(s)
     */
    virtual std::vector<armor_loc> get_hitbox(void) = 0;

private:
    /* to be added */
};

/**
 * A mod for assisted aiming; inspired by algorithm from DJI RoboRTS
 */
class ir_aimbot: public aimbot{
public:
    /**
     * constructor; initialize configuration into memory
     */
    ir_aimbot(CameraBase * cam_ptr, string color_type_str);

    /**
     * class destructor
     */
    ~ir_aimbot();

    /**
     * Process image (frame) in current video buffer; pure virtual function to be implemented
     * @return vector of cv::Rect object(s)
     */
    vector<armor_loc> get_hitbox(void);

    /**
     * @brief
     * @return get current frame updated by hit box (for debugging purpose only)
     */
    inline Mat & get_cur_frame(void) { return cur_frame; }

private:
    CameraBase * my_cam;

    Mat cur_frame;

    string my_color;

    int my_distillation_threshold;

    /**
     * @brief preprocess frame with magical methods
     * @param cur_frame_distilled ref to return value
     * @param cur_frame original image
     */
    void preprocess_frame(Mat & cur_frame_distilled, const Mat & cur_frame, Mat color_kernel, Mat gray_kernel);

    /**
     * @brief naive model of detecting light bars with findcontours
     * @param distilled_color
     * @param gray_bin binarized gray img
     * @return vector of cv::RotatedRect objects
     */
    vector<RotatedRect> detect_lights(Mat & distilled_color);

    /**
     * @brief naive filtering model
     * @param ori_img original image captured by camera
     * @param detect_lights detected light bars pulled from last step
     * @return vector of cv::RotatedRect objects
     */
    vector<RotatedRect> filter_lights(const Mat & ori_img, const vector<RotatedRect> & detected_light);

    /*
     * @brief naive armor detecting model based on armor angle, size, aspect ratio, light bar relative angles
     * @param filtered_light_bars filtered light bars
     * @param ori_img ref to original image captured by camera
     * @return vector of armor_loc objects
     */
    vector<armor_loc> detect_armor(vector<RotatedRect> & filtered_light_bars, const Mat & ori_img);

    /*
     * @brief armor filtering model. Not implemented for now
     * @param armor_obtained armor_loc objects from last step
     * @return filtered armor_loc objects
     */
    vector<armor_loc> filter_armor(const vector<armor_loc> & armor_obtained);
};

#endif
