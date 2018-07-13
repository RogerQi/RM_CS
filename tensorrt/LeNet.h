#ifndef _LENET_H_
#define _LENET_H_

#include "classifyNet.h"
#include "opencv2/opencv.hpp"
#include <vector>

#define DEFAULT_LENET_INPUT_NAME    "data"
#define DEFAULT_LENET_OUTPUT_NAME   "prob"

class LeNet: public classifyNet {
public:
    static LeNet* create(const char *prototxt_path,
            const char* model_path,
            const char *inp_name = DEFAULT_LENET_INPUT_NAME,
            const char *oup_name = DEFAULT_LENET_OUTPUT_NAME,
            uint32_t max_batchsize = 20);

    virtual ~LeNet();

    void preprocess(std::vector<cv::cuda::GpuMat> &imgs, 
            float *model_inps);
protected:
    LeNet();
};

#endif
