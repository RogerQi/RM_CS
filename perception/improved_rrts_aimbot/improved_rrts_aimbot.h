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
    *
    */
    vector<RotatedRect> detect_lights(Mat & distilled_color, Mat & gray_bin);

    /**
    *
    */
    vector<RotatedRect> filter_lights(const Mat & ori_img, const vector<RotatedRect> & detected_light);

    /*
    *
    */
    vector<armor_loc> detect_armor(vector<RotatedRect> & filtered_light_bars, const Mat & ori_img);

    /*
    *
    */
    vector<armor_loc> filter_armor(const vector<armor_loc> & armor_obtained);
};

#endif
