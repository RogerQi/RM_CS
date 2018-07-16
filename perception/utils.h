#ifndef UTILS_H_
#define UTILS_H_

#include <chrono>
#include <ratio>
#include <ctime>

class high_resolution_timer {
public:
    void start(void) {
        start_time = std::chrono::high_resolution_clock::now();
        cur_time = std::chrono::high_resolution_clock::now();
    }

    void cp(void) {
        cur_time = std::chrono::high_resolution_clock::now();
    }

    double get_duration(void) {
        std::chrono::duration<double> time_elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(cur_time - start_time);
        return time_elapsed.count();
    }

    std::chrono::high_resolution_clock::time_point get_cur_time(void) {
        return std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point cur_time;
};

#endif
