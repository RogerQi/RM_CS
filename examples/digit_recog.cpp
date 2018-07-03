#include <opencv2/opencv.hpp>
#include "camera.h"
#include "s_rune.h"

int main() {
    auto cam = new SimpleCVCam();
    auto rune = new s_rune();
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
        if(white_seq.size() > 0){
            for (auto w: white_seq)
                cout << w << ' ';
            cout << endl;
        }
        if (red_seq.size() > 0) {
            for (auto r: red_seq)
                cout << r << ' ';
            cout << endl;
        }
    }
    return 0;
}
