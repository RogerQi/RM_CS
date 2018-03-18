#include "camera.h"
#include "rune.h"
#include <opencv2/opencv.hpp>

int main() {
    auto cam = new SimpleCVCam();
    auto rune = new Rune();
    vector<int> white_seq;
    while (true) {
        rune->update(cam);
        rune->get_white_seq(white_seq);
    }
    return 0;
}
