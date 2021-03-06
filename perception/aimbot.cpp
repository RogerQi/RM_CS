#include "aimbot.h"
#include "utils.h"

static bool long_shoot = false;

template<class T> size_t get_target(const vector<T> & tar) {
    if(tar.size() == 0) return 0;
    size_t desired_tar = 0;
    float nearest_distance = sqrt(pow(tar[desired_tar].x - ORIG_IMAGE_WIDTH / 2, 2) + pow(tar[desired_tar].y - ORIG_IMAGE_HEIGHT / 2, 2));
    for(size_t i = 0; i < tar.size(); ++i) {
        float cur_distance = sqrt(pow(tar[i].x - ORIG_IMAGE_WIDTH / 2, 2) + pow(tar[i].y - ORIG_IMAGE_HEIGHT / 2, 2));
        if(cur_distance < nearest_distance) {
            desired_tar = i;
            nearest_distance = cur_distance;
        }
    }
    return desired_tar;
}

template<> size_t get_target<RotatedRect>(const vector<RotatedRect> & tar) {
    //@TODO: use a weighted algorithm
    if(tar.size() == 0) return 0;
    vector<Point2f> rect_pt (tar.size());
    for (size_t i = 0; i < tar.size(); ++i) rect_pt[i] = tar[i].center;
    return get_target(rect_pt);
}

template<> size_t get_target<armor_t>(const vector<armor_t> & tar) {
    if(tar.size() == 0) return 0;
    vector<RotatedRect> true_rect (tar.size());
    for (size_t i = 0; i < tar.size(); ++i) true_rect[i] = tar[i].armor;
    return get_target(true_rect);
}

Mat armor_perspective_transform(const Mat & src_img, const RotatedRect & roi){
    Mat cropped, M;
    Point2f dst_points[4] = {Point2f(0, 0), Point2f(0, 100),
                        Point2f(100, 0), Point2f(100, 100)};
    Point2f pts[4];
    roi.points(pts);
    if (pts[0].x > pts[1].x)
        swap(pts[0], pts[1]);
    if (pts[2].x > pts[3].x)
        swap(pts[2], pts[3]);
    M = getPerspectiveTransform(pts, dst_points);
    warpPerspective(src_img, cropped, M, Size(100, 100));
    cvtColor(cropped, cropped, COLOR_BGR2GRAY);
    return cropped;
}

void draw_rotated_rect(Mat & mat_to_draw, RotatedRect rect_to_draw) {
    Point2f rect_points[4];
    rect_to_draw.points(rect_points);
    for(int i = 0; i < 4; i++)
        cv::line(mat_to_draw, rect_points[i], rect_points[(i+1) % 4], Scalar(0, 255, 0), 1, 8);
}

float _cal_aspect_ratio(RotatedRect light) {
    Point2f rect_points[4];
    light.points(rect_points); //from opencv documentation: [bottomLeft, topLeft, topRight, bottomRight]
    sort(rect_points, rect_points + 4,
                [](const Point2f & a, const Point2f & b) { return a.x < b.x; });
    float width = rect_points[3].x - rect_points[0].x;
    sort(rect_points, rect_points + 4,
                [](const Point2f & a, const Point2f & b) { return a.y < b.y; });
    float height = rect_points[3].y - rect_points[0].y;
    return height / width;
}

Point2f _get_point_of_interest(const Mat & crop_distilled) {
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(crop_distilled, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
    if(contours.size() == 0)
        //no contours are found
        return Point2f(ORIG_IMAGE_WIDTH / 2, ORIG_IMAGE_HEIGHT / 2);

    vector<Moments> mu(contours.size());
    for(int i = 0; i < contours.size(); i++)
        mu[i] = moments(contours[i], false);


    ///  Get the mass centers:
    vector<Point2f> mc( contours.size() );
    for( int i = 0; i < contours.size(); i++ )
        mc[i] = Point2f(mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00);

    float dw = ORIG_IMAGE_WIDTH * 1.0 / crop_distilled.cols;
    float dh = ORIG_IMAGE_HEIGHT * 1.0 / crop_distilled.rows;
    Point2f nearest_pt = mc[get_target(mc)];
    nearest_pt.x *= dw;
    nearest_pt.y *= dh;
    return nearest_pt;
}

Mat _image_cropper(const Mat & frame, Point2f poi) {
    int lower_y = poi.y - IMAGE_HEIGHT / 2.0; //may be negative
    int upper_y = lower_y + IMAGE_HEIGHT;     //may overflow
    int left_x = poi.x - IMAGE_WIDTH / 2.0;   //may be negative
    int right_x = left_x + IMAGE_WIDTH;       //may overflow
    Mat ret(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);
    for(int x = left_x; x < right_x; x++) {
        int new_x = x - left_x;
        for(int y = lower_y; y < upper_y; y++) {
            int new_y = y - lower_y;
            if(x < 0 || x >= ORIG_IMAGE_WIDTH || y < 0 || y >= ORIG_IMAGE_HEIGHT) {
                //x or y is out of bound
                Vec3b color(0, 0, 0);
                ret.at<Vec3b>(Point(new_x, new_y)) = color;
            }
            else {
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
    if(my_color == "red")
        my_distillation_threshold = red_threshold;
}

ir_aimbot::~ir_aimbot(){
    //delete my_cam;
}

void ir_aimbot::preprocess_frame(Mat & cur_frame_distilled, const Mat & cur_frame, Mat color_kernel, Mat gray_kernel){
    cv::cuda::GpuMat cur_frame_gray_, cur_frame_gray_binarized, ret;
    cur_frame_gray_.upload(cur_frame);
    vector<cv::cuda::GpuMat> bgr;
    cv::cuda::split(cur_frame_gray_, bgr);
    if (my_color == "red") {
        distill_color_channel(bgr, 2, ret);
    } else {
        distill_color_channel(bgr, 0, ret);
    }
    cv::cuda::cvtColor(cur_frame_gray_, cur_frame_gray_, COLOR_BGR2GRAY);
    cv::cuda::threshold(cur_frame_gray_, cur_frame_gray_binarized, gray_threshold, 255, cv::THRESH_BINARY);
    cv::cuda::threshold(ret, ret, my_distillation_threshold, 255, cv::THRESH_BINARY);
    Ptr<cuda::Filter> color_filter = cv::cuda::createMorphologyFilter(cv::MORPH_DILATE,
        CV_8UC1, color_kernel);
    Ptr<cuda::Filter> gray_filter = cv::cuda::createMorphologyFilter(cv::MORPH_DILATE,
        CV_8UC1, gray_kernel);
    color_filter->apply(ret, ret);
    gray_filter->apply(cur_frame_gray_binarized, cur_frame_gray_binarized);
    cv::cuda::bitwise_and(ret, cur_frame_gray_binarized, ret);
    ret.download(cur_frame_distilled);
}

vector<armor_t> ir_aimbot::get_hitboxes(CameraBase * my_cam) {
    Point2f poi;
    //get cur image from camera
    Mat cur_frame_distilled;
    //my_cam->get_img(cur_frame);
    cur_frame = my_cam->cam_read();
    cur_frame.copyTo(ori_cur_frame);
    if (long_shoot) {
        Mat crop_detect, crop_distilled;
        resize(cur_frame, crop_detect, Size(320, 180));
#ifdef DEBUG
        imshow("Go1", crop_detect);
        waitKey(1);
#endif
    preprocess_frame(crop_distilled, crop_detect, Mat::ones(5, 5, CV_8UC1), Mat::ones(6, 6, CV_8UC1));
#ifdef DEBUG
    imshow("Crop_Distilled", crop_distilled);
    waitKey(1);
#endif
        poi = _get_point_of_interest(crop_distilled);
        /* process cur_frame (cropping) */
        cur_frame = _image_cropper(cur_frame, poi);
    } else {
        resize(cur_frame, cur_frame, Size(IMAGE_WIDTH, IMAGE_HEIGHT));
    }
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
    for(const RotatedRect & rect : light_bars)
        draw_rotated_rect(temp_img, rect);
    imshow("Light_bar", temp_img);
    waitKey(1);
#endif
    vector<armor_t> target_armors = detect_armor(light_bars, cur_frame);
    vector<armor_t> final_armor = filter_armor(target_armors);
    // correspond to original images size from camera
    for(size_t i = 0; i < final_armor.size(); ++i){
        if (long_shoot) {
            final_armor[i].armor.center.x -= IMAGE_WIDTH / 2;
            final_armor[i].armor.center.y -= IMAGE_HEIGHT / 2;
            final_armor[i].armor.center.x += poi.x;
            final_armor[i].armor.center.y += poi.y;
        } else {
            float width_scale_factor = ORIG_IMAGE_WIDTH * 1.0 / IMAGE_WIDTH;
            float height_scale_factor = ORIG_IMAGE_HEIGHT * 1.0 / IMAGE_HEIGHT;
            final_armor[i].armor.size.width *= width_scale_factor;
            final_armor[i].armor.size.height *= height_scale_factor;
            final_armor[i].armor.center.x *= width_scale_factor;
            final_armor[i].armor.center.y *= height_scale_factor;
            final_armor[i].left_light_bar.size.width *= width_scale_factor;
            final_armor[i].left_light_bar.size.height *= height_scale_factor;
            final_armor[i].left_light_bar.center.x *= width_scale_factor;
            final_armor[i].left_light_bar.center.y *= height_scale_factor;
            final_armor[i].right_light_bar.size.width *= width_scale_factor;
            final_armor[i].right_light_bar.size.height *= height_scale_factor;
            final_armor[i].right_light_bar.center.x *= width_scale_factor;
            final_armor[i].right_light_bar.center.y *= height_scale_factor;
        }
    }
    cur_visible_armors = final_armor;
    return final_armor;
}

aimbot_command_t ir_aimbot::get_desired_command(CameraBase *my_cam) {
    aimbot_command_t ret;
    vector<armor_t> visible_enemy_armors = get_hitboxes(my_cam);
    if (visible_enemy_armors.size() == 0) {
        ret.target_distance = 0;
    } else {
        size_t focus_armor_ind = get_target(visible_enemy_armors);
        ret.target_distance = get_armor_distance(visible_enemy_armors[focus_armor_ind]);
    }
    ret.abs_yaw = 0;
    ret.abs_pitch = 0;
    return ret;
}

float ir_aimbot::get_armor_distance(armor_t single_armor) {
    // based on navie camera optics model and measurement.
    float left_bar_height = max_of_two(single_armor.left_light_bar.size.height, single_armor.left_light_bar.size.width);
    float right_bar_height = max_of_two(single_armor.right_light_bar.size.height, single_armor.right_light_bar.size.width);
    float left_est = LIGHT_BAR_HEIGHT_ONE_METER / left_bar_height;
    float right_est = LIGHT_BAR_HEIGHT_ONE_METER / right_bar_height;
    return (left_est + right_est) / 2;
}

vector<RotatedRect> ir_aimbot::detect_lights(Mat & distilled_color){
    vector<vector<Point> > light_contours, gray_contours;
    vector<Vec4i> light_hierarchy, gray_hierarchy;
    findContours(distilled_color, light_contours, light_hierarchy, cnt_mode, cnt_method);
    //findContours(gray_bin, gray_contours, gray_hierarchy, cnt_mode, cnt_method);
    vector<RotatedRect> ret;
    for(const vector<Point> & cnt : light_contours)
        ret.push_back(minAreaRect(cnt));
    return ret;
}

vector<RotatedRect> ir_aimbot::filter_lights(const Mat & orig_img,
        const vector<RotatedRect> & detected_light) {
    vector<RotatedRect> ret;
    for(const RotatedRect &light : detected_light) {
        float angle = 0.0f;
        float light_aspect_ratio = _cal_aspect_ratio(light);
        angle = light.angle >= 90.0 ? std::abs(light.angle - 90.0) : std::abs(light.angle);
        //std::cout << "current light bar ratio" << light_aspect_ratio << std::endl;
        if ((light_aspect_ratio < light_max_aspect_ratio && light_aspect_ratio > light_min_apsect_ratio) &&
                    (light.size.area() >= light_min_area)) {
            //calculate avg value of the specific channel
            //Mat & this_light_bar = ori_img()
            ret.push_back(light);
        }
    }
    return ret;
}

vector<armor_t> ir_aimbot::detect_armor(vector<RotatedRect> & filtered_light_bars, const Mat & ori_img) {
    vector<armor_t> ret;
    for(size_t i = 0; i < filtered_light_bars.size(); i++) {
        RotatedRect light_1 = filtered_light_bars[i];
        for(size_t j = i + 1; j < filtered_light_bars.size(); j++) {
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
            if(y_offset_sq < 400 &&   //y offset can be too far
               bbox_w * bbox_h > 40 &&
               bbox_w / bbox_h < 6) {
                RotatedRect this_armor;
                armor_t this_armor_with_light_bars;
                this_armor.center = Point2f(bbox_x, bbox_y);
                this_armor.size = Size2f(bbox_w, bbox_h);
                this_armor.angle = bbox_angle;
                this_armor_with_light_bars.armor = this_armor;
                if (light_1.center.x > light_2.center.x) {
                    this_armor_with_light_bars.right_light_bar = light_1;
                    this_armor_with_light_bars.left_light_bar = light_2;
                } else {
                    this_armor_with_light_bars.right_light_bar = light_2;
                    this_armor_with_light_bars.left_light_bar = light_1;
                }
                ret.push_back(this_armor_with_light_bars);
            }
        }
    }
    return ret;
}

vector<armor_t> ir_aimbot::filter_armor(const vector<armor_t> & armor_obtained){
    vector<armor_t> filtered;
    for (size_t i = 0; i < armor_obtained.size(); ++i)
        filtered.push_back(armor_obtained[i]);
    return filtered;
}
