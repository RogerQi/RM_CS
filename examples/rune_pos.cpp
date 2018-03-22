#include <opencv2/opencv.hpp>
#include "camera.h"
#include "rune.h"
#include "cv_config.h"
#include <unistd.h>

int main() {
    auto cam = new SimpleCVCam();
    auto rune = new Rune();
    while (true) {
        int pos = rune->get_hit_pos(cam);
        std::cout << "Hitting " << pos << std::endl;
        usleep(RUNE_WAIT_TIME);
    }
    return 0;
}
