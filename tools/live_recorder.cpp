#include "camera.h"
#include "cv_config.h"
#include "camera_saver.h"
#include <thread>
#include <chrono>

void record_loop() {
    SimpleCVCam *cam = new SimpleCVCam(0);
    video_saver my_saver(cam, 500);

    while (true) {
        my_saver.save_image();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main(void) {
    std::thread t(record_loop);
    t.detach();

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(5));
}

