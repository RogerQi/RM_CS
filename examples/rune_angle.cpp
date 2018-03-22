#include <opencv2/opencv.hpp>
#include "camera.h"
#include "rune.h"
#include "cv_config.h"
#include <unistd.h>

int main() {
    auto cam = new SimpleCVCam();
    auto rune = new Rune();
    while (true) {
        pair<float, float> pos = rune->get_hit_angle(cam);
        std::cout << "Yaw: " << pos.second << std::endl;
        std::cout << "Pitch: " << pos.first << std::endl;
        usleep(RUNE_WAIT_TIME);
    }
    return 0;
}
