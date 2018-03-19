#ifndef IR_AIMBOT_H_
#define IR_AIMBOT_H_

#include "aimbot.h" //opencv included
#include "improved_rrts_aimbot/improved_rrts_aimbot_config.h"
#include "camera.h"
#include <string>
#include <algorithm>
#include <math.h>

using std::string;

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
    vector<armor_loc> get_hitbox(void) const;

private:
    CameraBase * my_cam;

    Mat cur_frame;

    string my_color;

    int my_distillation_threshold;

    /**
    * @brief naive model of detecting light bars with findcontours
    * @param distilled_color
    * @param gray_bin binarized gray img
    * @return vector of cv::ROtatedRect objects
    */
    vector<RotatedRect> detect_lights(Mat & distilled_color, Mat & gray_bin);

    /**
    * @brief naive filtering model
    * @param ori_img original image captured by camera
    * @param detect_lights detected light bars pulled from last step
    * @return vector of cv::ROtatedRect objects
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
