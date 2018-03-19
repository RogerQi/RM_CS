#include "improved_rrts_aimbot/improved_rrts_aimbot.h"
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
        for(size_t i = 0; i < ret.size(); i++){
            std::cout << "Cur armor center: (" << ret[i].center_x << ", " << ret[i].center_y << ")" << std::endl;
        }
    }
    return 0;
}
