#include "aimbot.h"
#include "video_feeder.h"
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <sstream>

using std::string;

int counter = 0;

/* code for affine transform */

/*
float angle = the_rotated_rect.angle;
Size rect_size = the_rotated_rect.size;
if(angle < -45){
    angle += 90.0;
    std::swap(rect_size.width, rect_size.height);
}
Mat M, rotated, cropped;
M = getRotationMatrix2D(the_rotated_rect.center, angle, 1.0);
warpAffine(a_frame, rotated, M, a_frame.size(), INTER_CUBIC);
getRectSubPix(rotated, rect_size, the_rotated_rect.center, cropped);
*/

void save_rotated_rect(const Mat & a_frame, const RotatedRect & the_rotated_rect){
    Mat cropped, M;
    Point2f dst_points[4] = {Point2f(0, 0), Point2f(0, 100),
                        Point2f(100, 0), Point2f(100, 100)};
    Point2f pts[4];
    the_rotated_rect.points(pts);
    if (pts[0].x > pts[1].x)
        swap(pts[0], pts[1]);
    if (pts[2].x > pts[3].x)
        swap(pts[2], pts[3]);
    M = getPerspectiveTransform(pts, dst_points);
    warpPerspective(a_frame, cropped, M, Size(100, 100));
    std::stringstream ss;
    ss << counter << ".jpg";
    counter++;
    imwrite(ss.str(), cropped);
}

void make_data(string file_path){
    CameraBase * cam = new VideoFeed(file_path);
    ir_aimbot detector("blue");
    int iter = -1;
    while(cam->is_alive()){
        iter++;
        std::vector<RotatedRect> ret = detector.get_hitbox(cam);
        Mat frame_shower = detector.get_cur_frame();
        if(iter % 16 != 0){
            continue;
        }
        for(const RotatedRect & rr: ret){
            save_rotated_rect(frame_shower, rr);
        }
    }
    return;
}

int main(void){
    int iter = 1;
    while(iter < 13){
        std::stringstream cur_path;
        cur_path << iter << ".mp4";
        make_data(cur_path.str());
    }
    iter++;
}
