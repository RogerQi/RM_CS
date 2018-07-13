#ifndef _CLASSIFYNET_H_
#define _CLASSIFYNET_H_

#include "tensorNet.h"
#include "opencv2/opencv.hpp"
#include <vector>

class classifyNet: public tensorNet {
public:
    static classifyNet* create(const char* prototxt_path, const char* model_path,
            const uint32_t output_dim,
            const char* inp_name,
            const char* oup_name,
            uint32_t max_batchsize = 32);
    virtual ~classifyNet();

    bool predict(std::vector<cv::cuda::GpuMat> &imgs,
            std::vector<uint8_t> *pred = NULL,
            std::vector<float> *confidence = NULL);

    virtual void preprocess(std::vector<cv::cuda::GpuMat> &imgs,
            float *model_inps);
    
protected:
    classifyNet();

    bool init(const char* prototxt_path, const char* model_path,
            const char* inp_name, const char* oup_name,
            uint32_t max_batchsize);

    uint32_t output_dim;
};

#endif
