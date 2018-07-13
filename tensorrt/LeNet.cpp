#include "LeNet.h"
#include "cudaUtility.h"

cudaError_t cudaLeNetNormalize(cv::cuda::GpuMat &input, float *output);

LeNet::LeNet() : classifyNet() {

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
        printf("lenet failed to initialize.\n");
        delete net;
        return NULL;
    }

    net->output_dim = 11;

    return net;
}

void LeNet::preprocess(std::vector<cv::cuda::GpuMat> &imgs,
        float *model_inps) {
    for (size_t i = 0; i < imgs.size(); i++) {
        cudaLeNetNormalize(imgs[i], model_inps + i * 28 * 28);
    }
}
