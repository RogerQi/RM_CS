#include "camera.h"
#include <opencv2/opencv.hpp>
#include <thread>

using namespace std;
using namespace cv;

int main() {
    SimpleCVCam cam;
    Mat frame;
    while (true) {
        cam.get_img(frame);
        imshow("go", frame); 
        waitKey(1);
    }
    return 0;
}
