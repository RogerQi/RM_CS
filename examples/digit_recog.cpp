#include <opencv2/opencv.hpp>
#include "camera.h"
#include "rune.h"

int main() {
    auto cam = new SimpleCVCam();
    auto rune = new Rune();
    while (true) {
        vector<int> white_seq;
        rune->update(cam);
        rune->get_white_seq(white_seq);
#ifdef DEBUG
        for (auto w: white_seq)
            cout << w << ' ';
        cout << endl;
#endif
    }
    return 0;
}
