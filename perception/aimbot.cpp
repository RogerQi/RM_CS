#include "aimbot.h"
#include <caffe/caffe.hpp>
#include <caffe/blob.hpp>
#include <caffe/util/io.hpp>

void distill_color(const Mat & src_img, Mat & dst_img, string color_type) {
    std::vector<cv::Mat> bgr;
    cv::split(src_img, bgr);
    if (color_type == "red") {
        cv::subtract(bgr[2], bgr[1], dst_img);
    } else {
        //assume it's blue
        cv::subtract(bgr[0], bgr[2], dst_img);
    }
}

void draw_rotated_rect(Mat & mat_to_draw, RotatedRect rect_to_draw){
    Point2f rect_points[4];
    rect_to_draw.points(rect_points);
    for(int i = 0; i < 4; i++){
        line(mat_to_draw, rect_points[i], rect_points[(i+1) % 4], Scalar(0, 255, 0), 1, 8);
    }
}

float _cal_aspect_ratio(RotatedRect light){
    Point2f * rect_points = new Point2f[4];
    light.points(rect_points); //from opencv documentation: [bottomLeft, topLeft, topRight, bottomRight]
    sort(rect_points, rect_points + 4,
                [](const Point2f & a, const Point2f & b) {return a.x < b.x; });
    float width = rect_points[3].x - rect_points[0].x;
    sort(rect_points, rect_points + 4,
                [](const Point2f & a, const Point2f & b) {return a.y < b.y; });
    float height = rect_points[3].y - rect_points[0].y;
    delete[] rect_points;
    return height / width;
    /*
    float height = sqrt(pow(rect_points[1].x - rect_points[0].x, 2)
                                + pow(rect_points[1].y - rect_points[0].y, 2));
    float width = sqrt(pow(rect_points[1].x - rect_points[2].x, 2)
                                + pow(rect_points[1].y - rect_points[2].y, 2));
    return height / width;
    */
}

Point2f _get_point_of_interest(const Mat & crop_distilled){
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Point2f middle_pt(ORIG_IMAGE_WIDTH * 1.0 / 2, ORIG_IMAGE_HEIGHT * 1.0 / 2);
    findContours(crop_distilled, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    if(contours.size() == 0){
        //no contours are found
        return middle_pt;
    }
    vector<Moments> mu(contours.size());
    for(int i = 0; i < contours.size(); i++){
        mu[i] = moments(contours[i], false);
    }

    ///  Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( int i = 0; i < contours.size(); i++ ){
        mc[i] = Point2f(mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00);
    }
    float dw = ORIG_IMAGE_WIDTH * 1.0 / crop_distilled.cols;
    float dh = ORIG_IMAGE_HEIGHT * 1.0 / crop_distilled.rows;
    Point2f nearest_pt = mc[0];
    float nearest_distance = sqrt(pow(nearest_pt.x - middle_pt.x, 2) + pow(nearest_pt.y - middle_pt.y, 2));
    for(const Point2f pt : mc){
        float cur_distance = sqrt(pow(pt.x - middle_pt.x, 2) + pow(pt.y - middle_pt.y, 2));
        if(cur_distance < nearest_distance){
            nearest_pt = pt;
            nearest_distance = cur_distance;
        }
    }
    nearest_pt.x *= dw;
    nearest_pt.y *= dh;
    return nearest_pt;
}

Mat _image_cropper(const Mat & frame, Point2f poi){
    int lower_y = poi.y - IMAGE_HEIGHT / 2.0; //may be negative
    int upper_y = lower_y + IMAGE_HEIGHT; //may be overflow
    int left_x = poi.x - IMAGE_WIDTH / 2.0; //may be negative
    int right_x = left_x + IMAGE_WIDTH; //may be overflow
    Mat ret(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
    for(int x = left_x; x < right_x; x++){
        int new_x = x - left_x;
        for(int y = lower_y; y < upper_y; y++){
            int new_y = y - lower_y;
            if(x < 0 || x >= ORIG_IMAGE_WIDTH || y < 0 || y >= ORIG_IMAGE_HEIGHT){
                //x or y is out of bound
                Vec3b color(0, 0, 0);
                ret.at<Vec3b>(Point(new_x, new_y)) = color;
            } else {
                Vec3b color = frame.at<Vec3b>(Point(x, y));
                ret.at<Vec3b>(Point(new_x, new_y)) = color;
            }
        }
    }
    return ret;
}

ir_aimbot::ir_aimbot(string color_type_str){
    my_color = color_type_str;
    my_distillation_threshold = blue_threshold;
    if(my_color == "red"){
        my_distillation_threshold = red_threshold;
    }
}

ir_aimbot::~ir_aimbot(){
    //delete my_cam;
}

void ir_aimbot::preprocess_frame(Mat & cur_frame_distilled, const Mat & cur_frame, Mat color_kernel, Mat gray_kernel){
    Mat cur_frame_gray_, cur_frame_gray_binarized;
    cvtColor(cur_frame, cur_frame_gray_, COLOR_BGR2GRAY);
    threshold(cur_frame_gray_, cur_frame_gray_binarized, gray_threshold, 255, THRESH_BINARY);
    distill_color(cur_frame, cur_frame_distilled, my_color);
    threshold(cur_frame_distilled, cur_frame_distilled, my_distillation_threshold, 255, THRESH_BINARY);
    dilate(cur_frame_distilled, cur_frame_distilled, color_kernel, Point(-1, -1), 1);
    dilate(cur_frame_gray_binarized, cur_frame_gray_binarized, gray_kernel, Point(-1, -1), 1);
    cur_frame_distilled = cur_frame_distilled & cur_frame_gray_binarized;
}

vector<RotatedRect> ir_aimbot::get_hitbox(CameraBase * my_cam){
    //get cur image from camera
    Mat cur_frame_distilled;
    my_cam->get_img(cur_frame);
    Mat crop_detect, crop_distilled;
    resize(cur_frame, crop_detect, Size(320, 180));
    #ifdef DEBUG
        imshow("Go1", crop_detect);
        waitKey(1);
    #endif
    preprocess_frame(crop_distilled, crop_detect, Mat::ones(10, 10, CV_8UC1), Mat::ones(12, 12, CV_8UC1));
    #ifdef DEBUG
        imshow("Crop_Distilled", crop_distilled);
        waitKey(1);
    #endif
    Point2f poi = _get_point_of_interest(crop_distilled);
    /* process cur_frame (cropping) */
    cur_frame = _image_cropper(cur_frame, poi);
    /* begin color distillation; pulled from RoboRTS */
    preprocess_frame(cur_frame_distilled, cur_frame,
            Mat::ones(light_bar_kernel_height, light_bar_kernel_width, CV_8UC1),
            Mat::ones(gray_bin_kernel_height, gray_bin_kernel_width, CV_8UC1));
    /* end color distillation; cur_frame_distilled should be clean and steady */
    vector<RotatedRect> light_bars = detect_lights(cur_frame_distilled);
    light_bars = filter_lights(cur_frame, light_bars);
    //show captured light bars for debugging
    #ifdef DEBUG
        Mat temp_img;
        cur_frame.copyTo(temp_img);
        for(const RotatedRect & rect : light_bars){
            draw_rotated_rect(temp_img, rect);
        }
        imshow("Light_bar", temp_img);
        waitKey(1);
    #endif
    vector<RotatedRect> target_armors = detect_armor(light_bars, cur_frame);
    return filter_armor(target_armors);
}

vector<RotatedRect> ir_aimbot::detect_lights(Mat & distilled_color){
    vector<vector<Point> > light_contours, gray_contours;
    vector<Vec4i> light_hierarchy, gray_hierarchy;
    findContours(distilled_color, light_contours, light_hierarchy, cnt_mode, cnt_method);
    //findContours(gray_bin, gray_contours, gray_hierarchy, cnt_mode, cnt_method);
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
        float light_aspect_ratio = _cal_aspect_ratio(light);
        angle = light.angle >= 90.0 ? std::abs(light.angle - 90.0) : std::abs(light.angle);
        //std::cout << "current light bar ratio" << light_aspect_ratio << std::endl;
        if ((light_aspect_ratio < light_max_aspect_ratio && light_aspect_ratio > light_min_apsect_ratio) &&
                    (light.size.area() >= light_min_area)){
            //calculate avg value of the specific channel
            //Mat & this_light_bar = ori_img()
            ret.push_back(light);
        }
    }
    return ret;
}

vector<RotatedRect> ir_aimbot::detect_armor(vector<RotatedRect> & filtered_light_bars, const Mat & ori_img){
    vector<RotatedRect> ret;
    for(size_t i = 0; i < filtered_light_bars.size(); i++){
        RotatedRect light_1 = filtered_light_bars[i];
        for(size_t j = i + 1; j < filtered_light_bars.size(); j++){
            RotatedRect light_2 = filtered_light_bars[j];
            /*
            if(fabs(light_1.angle - light_2.angle) > light_max_angle_diff){
                continue;
            }
            */
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
            if(y_offset_sq < 400){ //y offset can be too far
                if(bbox_w * bbox_h > 40){
                    if(bbox_w / bbox_h < 6){
                        RotatedRect this_armor;
                        this_armor.center = Point2f(bbox_x, bbox_y);
                        this_armor.size = Size2f(bbox_w, bbox_h);
                        this_armor.angle = bbox_angle;
                        ret.push_back(this_armor);
                    }
                }
            }
            /*
            if(fabs(bbox_angle) < armor_max_angle){ //constraint 1: armor angle
                if((bbox_w / bbox_h) < armor_max_aspect_ratio){ //constraint 2: armor aspect ratio
                    if((bbox_w * bbox_h) > armor_min_area){ //constraint 3: armor can't be too small
                    RotatedRect this_armor;
                    this_armor.center = Point2f(bbox_x, bbox_y);
                    this_armor.size = Size2f(bbox_w, bbox_h);
                    this_armor.angle = bbox_angle;
                    ret.push_back(this_armor);
                    }
                }
            }
            */
        }
    }
    return ret;
}

vector<RotatedRect> ir_aimbot::filter_armor(const vector<RotatedRect> & armor_obtained){
    //do nothing yet
    return armor_obtained;
}
