#include "cudaUtility.h"
#include "opencv2/opencv.hpp"

__global__ void gpuPreLeNetNormalize(cv::cuda::PtrStepSzb input, float* output) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    output[y * 28 + x] = input(y, x) / 255.0;
}

cudaError_t cudaPreLeNetNormalize(cv::cuda::GpuMat &input, float* output) {
    const dim3 blockDim(7, 7);
    const dim3 gridDim(4, 4);

    gpuPreLeNetNormalize<<<gridDim, blockDim>>>(input, output);

    return CUDA(cudaGetLastError());
}
