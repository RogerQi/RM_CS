#include "LeNet.h"
#include "cudaMappedMemory.h"
#include "cudaResize.h"

cudaError_t cudaPreLeNetNormalize(cv::cuda::GpuMat &input, float* output);

LeNet::LeNet() : tensorNet() {

}

LeNet::~LeNet() {
    
}

LeNet* LeNet::create(const char *prototxt_path, const char *model_path,
            const char *inp_name, const char *oup_name,
            uint32_t max_batchsize) {
    LeNet *net = new LeNet();

    if (!net)
        return NULL;

    if (!net->init(prototxt_path, model_path, inp_name, oup_name, max_batchsize)) {
        printf("lenet failed to intialize.\n");
        return NULL;
    }

    return net;
}

bool LeNet::init(const char *prototxt_path, const char *model_path,
        const char *inp_name, const char *oup_name,
        uint32_t max_batchsize) {
    if (!prototxt_path || !model_path || !inp_name || !oup_name)
        return false;

    if (!tensorNet::LoadNetwork(prototxt_path, model_path, NULL, inp_name, oup_name, max_batchsize)) {
        printf("failed to load %s\n", model_path);
        return false;
    }

    return true;
}

bool LeNet::predict(std::vector<cv::cuda::GpuMat> &imgs,
        std::vector<uint8_t> *pred,
        std::vector<float> *confidence) {
    for (size_t i = 0; i < imgs.size(); i++) {
        if (CUDA_FAILED(cudaPreLeNetNormalize(imgs[i],
                        mInputCUDA + i * 28*28))) {
            printf("lenet::preidic() -- cudaPreLeNetCopy failed\n");
            return false;
        }
    }

    void *inferenceBuffers[] = { mInputCUDA, mOutputs[0].CUDA };
    
    mContext->execute(imgs.size(), inferenceBuffers);

    PROFILER_REPORT();

    int class_idx = -1;
    float class_max = -1.0f;

    if (pred) pred->resize(imgs.size());
    if (confidence) confidence->resize(imgs.size());

    for (size_t n = 0; n < imgs.size(); n++) {
        class_idx = class_max = -1;
        for (size_t i = 0; i < 11; i++) {
            const float value = mOutputs[0].CPU[n * 11 + i];
            if (value > class_max) {
                class_idx = i;
                class_max = value;
            }
        }
        if (pred) pred->at(n) = (uint8_t)class_idx;
        if (confidence) confidence->at(n) = class_max;
    }
    
    return true;
}
