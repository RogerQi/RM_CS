#include "improved_rrts_aimbot.h"

template<class T>
const T& max_of_two(const T& a, const T&b) {return (a>b)? a:b;}

ir_aimbot::ir_aimbot(CameraBase * cam_ptr, string color_type_str) : my_cam(cam_ptr){
    my_color = color_type_str;
    my_distillation_threshold = blue_threshold;
    if(my_color == "red"){
        my_distillation_threshold = red_threshold;
    }
}

ir_aimbot::~ir_aimbot(){
    //do nothing for now
}

vector<armor_loc> ir_aimbot::get_hitbox(void){
    //get cur image from camera
    Mat cur_frame_gray_, cur_frame_gray_binarized, cur_frame_distilled;
    my_cam->get_img(cur_frame);
    //preprocess cur frame such that we get a black and white copy
    /* begin color distillation; pulled from RoboRTS */
    cvtColor(cur_frame, cur_frame_gray_, COLOR_BGR2GRAY);
    threshold(cur_frame_gray_, cur_frame_gray_binarized, gray_threshold, 255, THRESH_BINARY);
    distill_color(cur_frame, cur_frame_distilled, my_color);
    threshold(cur_frame_distilled, cur_frame_distilled, my_distillation_threshold, 255, THRESH_BINARY);
    dilate(cur_frame_distilled, cur_frame_distilled, Mat::ones(light_bar_kernel_height, light_bar_kernel_width, CV_8UC1), Point(-1, -1), light_bar_kernel_iter);
    dilate(cur_frame_gray_binarized, cur_frame_gray_binarized, Mat::ones(gray_bin_kernel_height, gray_bin_kernel_width, CV_8UC1), Point(-1, -1), light_bar_kernel_iter);
    cur_frame_distilled = cur_frame_distilled & cur_frame_gray_binarized;
    #ifdef DEBUG
        imshow("Distilled", cur_frame_distilled);
        waitKey(1);
    #endif
    /* end color distillation; cur_frame_distilled should be clean and steady */
    vector<RotatedRect> light_bars = detect_lights(cur_frame_distilled, cur_frame_gray_binarized);
    light_bars = filter_lights(cur_frame, light_bars);
    vector<armor_loc> target_armors = detect_armor(light_bars, cur_frame);
    return filter_armor(target_armors);
}

vector<RotatedRect> ir_aimbot::detect_lights(Mat & distilled_color, Mat & gray_bin){
    vector<vector<Point> > light_contours, gray_contours;
    vector<Vec4i> light_hierarchy, gray_hierarchy;
    findContours(distilled_color, light_contours, light_hierarchy, cnt_mode, cnt_method);
    findContours(gray_bin, gray_contours, gray_hierarchy, cnt_mode, cnt_method);
    vector<RotatedRect> ret;
    for(const vector<Point> & cnt : light_contours){
        ret.push_back(minAreaRect(cnt));
    }
    return ret;
}

vector<RotatedRect> ir_aimbot::filter_lights(const Mat & orig_img, const vector<RotatedRect> & detected_light){
    vector<RotatedRect> ret;
    for(const RotatedRect &light : detected_light){
        float angle = 0.0f;
        float light_aspect_ratio =
                std::max(light.size.width, light.size.height) / std::min(light.size.width, light.size.height);
        angle = light.angle >= 90.0 ? std::abs(light.angle - 90.0) : std::abs(light.angle);
        if (light_aspect_ratio < light_max_aspect_ratio ||
                    (angle < light_max_angle && light.size.area() >= light_min_area)){
            //calculate avg value of the specific channel
            //Mat & this_light_bar = ori_img()
            ret.push_back(light);
        }
    }
    return ret;
}

vector<armor_loc> ir_aimbot::detect_armor(vector<RotatedRect> & filtered_light_bars, const Mat & ori_img){
    vector<armor_loc> ret;
    for(size_t i = 0; i < filtered_light_bars.size(); i++){
        RotatedRect light_1 = filtered_light_bars[i];
        for(size_t j = i + 1; j < filtered_light_bars.size(); j++){
            RotatedRect light_2 = filtered_light_bars[j];
            if(fabs(light_1.angle - light_2.angle) > light_max_angle_diff){
                continue;
            }
            float light_1_height = max_of_two(light_1.size.height, light_1.size.width);
            float light_2_height = max_of_two(light_2.size.height, light_2.size.width);
            float bbox_h = max_of_two(light_1_height, light_2_height);
            float x_offset_sq = pow(light_1.center.x - light_2.center.x, 2);
            float y_offset_sq = pow(light_1.center.y - light_2.center.y, 2);
            float light_dis = sqrt(x_offset_sq + y_offset_sq);
            float x_diff = light_1.center.x - light_2.center.x;
            if(fabs(x_diff) < 1e-9)
                x_diff = 1e-9; //to avoid division by zero error
            float bbox_angle = atan((light_1.center.y - light_2.center.y) / x_diff);
            float bbox_x = (light_1.center.x + light_2.center.x) / 2.0;
            float bbox_y = (light_1.center.y + light_2.center.y) / 2.0;
            float bbox_w = light_dis * 1.2;
            if(fabs(bbox_angle) < armor_max_angle){
                if((bbox_w / bbox_h) < armor_max_aspect_ratio){
                    if((bbox_w * bbox_h) > armor_min_area){
                        armor_loc this_armor;
                        this_armor.center_x = bbox_x;
                        this_armor.center_y = bbox_y;
                        this_armor.ang = bbox_angle;
                        this_armor.width = bbox_w;
                        this_armor.height = bbox_h;
                        ret.push_back(this_armor);
                    }
                }
            }
        }
    }
    return ret;
}

vector<armor_loc> ir_aimbot::filter_armor(const vector<armor_loc> & armor_obtained){
    //do nothing yet
    return armor_obtained;
}
