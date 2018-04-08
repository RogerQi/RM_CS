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
    ir_aimbot detector("blue");
    int code = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    cv::VideoWriter save_stream("rendered.avi", code, 30, Size(1920, 1080));
    while(cam->is_alive()){
        std::vector<RotatedRect> ret = detector.get_hitbox(cam);
        Mat frame_shower = detector.get_cur_frame();
        std::vector<RotatedRect> visual_rects;
        visual_rects.reserve(ret.size());
        for(size_t i = 0; i < ret.size(); i++){
            visual_rects.push_back(ret[i]);
        }
        Scalar color = Scalar(255, 0, 0);
        for(size_t i = 0; i < visual_rects.size(); i++){
            ellipse(frame_shower, visual_rects[i], color, 2, 8);
            //std::cout << "Cur armor center: (" << ret[i].center_x << ", " << ret[i].center_y << ")" << std::endl;
        }
        save_stream << frame_shower;
        imshow("Go", frame_shower);
        waitKey(1);
    }
    save_stream.release();
    return 0;
}
