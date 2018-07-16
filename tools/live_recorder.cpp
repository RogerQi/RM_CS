#include "camera.h"
#include "cv_config.h"
#include "camera_saver.h"
#include <thread>
#include <chrono>

int main(void) {
    OV5693Cam *tx2_cam = new OV5693Cam();
    video_saver my_saver(tx2_cam, 500);
    while (true) {
        // two threads should already be working now
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
