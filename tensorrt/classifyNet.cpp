#include "classifyNet.h"
#include "cudaMappedMemory.h"
#include "cudaResize.h"

cudaError_t cudaImgCopy(cv::cuda::GpuMat &input, float* output, 
        uint32_t height, uint32_t width);

classifyNet::classifyNet() : tensorNet() {

}

classifyNet::~classifyNet() {
    
}

classifyNet* classifyNet::create(const char *prototxt_path, const char *model_path,
            const uint32_t output_dim, const char *inp_name, const char *oup_name, 
            uint32_t max_batchsize) {
    classifyNet *net = new classifyNet();

    if (!net)
        return NULL;

    if (!net->init(prototxt_path, model_path, inp_name, oup_name, max_batchsize)) {
        printf("lenet failed to intialize.\n");
        delete net;
        return NULL;
    }

    net->output_dim = output_dim;

    return net;
}

bool classifyNet::init(const char *prototxt_path, const char *model_path,
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

bool classifyNet::predict(std::vector<cv::cuda::GpuMat> &imgs,
        std::vector<uint8_t> *pred,
        std::vector<float> *confidence) {

    preprocess(imgs, mInputCUDA);
    void *inferenceBuffers[] = { mInputCUDA, mOutputs[0].CUDA };
    
    mContext->execute(imgs.size(), inferenceBuffers);

    PROFILER_REPORT();

    int class_idx = -1;
    float class_max = -1.0f;

    if (pred) pred->resize(imgs.size());
    if (confidence) confidence->resize(imgs.size());

    for (size_t n = 0; n < imgs.size(); n++) {
        class_idx = class_max = -1;
        for (size_t i = 0; i < output_dim; i++) {
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

void classifyNet::preprocess(std::vector<cv::cuda::GpuMat> &imgs,
        float *model_inps) {
    uint32_t height = imgs[0].rows;
    uint32_t width = imgs[0].cols;
    for (size_t i = 0; i < imgs.size(); i++) {
        cudaImgCopy(imgs[i], model_inps + i * height * width, height, width);
    }
}
