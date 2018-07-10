#ifndef _LENET_H_
#define _LENET_H_

#include "tensorNet.h"
#include "opencv2/opencv.hpp"
#include <vector>

#define DEFAULT_LENET_INPUT_NAME    "data"
#define DEFAULT_LENET_OUTPUT_NAME   "prob"

class LeNet: public tensorNet {
public:
    static LeNet* create(const char* prototxt_path, const char* model_path,
            const char* inp_name = DEFAULT_LENET_INPUT_NAME,
            const char* oup_name = DEFAULT_LENET_OUTPUT_NAME,
            uint32_t max_batchsize = 32);
    virtual ~LeNet();

    bool predict(std::vector<cv::cuda::GpuMat> &imgs,
            std::vector<uint8_t> *pred = NULL,
            std::vector<float> *confidence = NULL);
    
protected:
    LeNet();

    bool init(const char* prototxt_path, const char* model_path,
            const char* inp_name, const char* oup_name,
            uint32_t max_batchsize);
};

#endif
