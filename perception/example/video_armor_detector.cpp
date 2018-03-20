#include "aimbot.h"
#include "video_feeder.h"
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

using std::string;

int main(void){
    string file_path = "blue_1.mp4";
    CameraBase * cam = new VideoFeed(file_path);
    ir_aimbot detector(cam, "blue");
    while(cam->is_alive()){
        std::vector<armor_loc> ret = detector.get_hitbox();
        Mat frame_shower = detector.get_cur_frame();
        std::vector<RotatedRect> visual_rects;
        visual_rects.reserve(ret.size());
        for(size_t i = 0; i < ret.size(); i++){
            visual_rects.push_back(armor_loc_2_rotated_rect(ret[i]));
        }
        Scalar color = Scalar(255, 0, 0);
        for(size_t i = 0; i < visual_rects.size(); i++){
            ellipse(frame_shower, visual_rects[i], color, 2, 8);
            //std::cout << "Cur armor center: (" << ret[i].center_x << ", " << ret[i].center_y << ")" << std::endl;
        }
        //imshow("Go", frame_shower);
        waitKey(1);
    }
    return 0;
}
