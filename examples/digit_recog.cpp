#include <opencv2/opencv.hpp>
#include "camera.h"
#include "rune.h"

int main() {
    auto cam = new SimpleCVCam();
    auto rune = new Rune();
    while (true) {
        vector<int> white_seq;
        rune->update(cam);
        bool success;
        success = rune->get_white_seq(white_seq);
        if(!(success)){
            continue;
        }
        vector<int> red_seq;
        rune->get_red_seq(red_seq);
#ifdef DEBUG
        if(white_seq.size() > 0){
            for (auto w: white_seq)
                cout << w << ' ';
            cout << endl;
        }
#endif
    }
    return 0;
}
