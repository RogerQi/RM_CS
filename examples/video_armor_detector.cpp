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
    cv::VideoWriter save_stream("rendered.avi", code, 30, Size(640, 360));
    while(cam->is_alive()){
        aimbot_command_t cur_command = detector.get_desired_command(cam);
        std::vector<armor_t> ret = detector.get_latest_visible_armors();
        Mat frame_shower = detector.get_cur_frame();
        std::vector<RotatedRect> visual_rects;
        visual_rects.reserve(ret.size());
        for(size_t i = 0; i < ret.size(); i++){
            visual_rects.push_back(ret[i].armor);
        }
        Scalar color = Scalar(255, 0, 0);
        for(size_t i = 0; i < visual_rects.size(); i++){
            draw_rotated_rect(frame_shower, visual_rects[i]);
            //std::cout << "Cur armor center: (" << ret[i].center_x << ", " << ret[i].center_y << ")" << std::endl;
        }
        putText(frame_shower, std::to_string(cur_command.target_distance), cvPoint(100, 100),
                        FONT_HERSHEY_SIMPLEX, 0.9, Scalar(0, 150, 100), 2, LINE_AA);
        save_stream << frame_shower;
        //resize(frame_shower, frame_shower, Size(640, 360));
        //imshow("Go", frame_shower);
        waitKey(1);
    }
    save_stream.release();
    return 0;
}
