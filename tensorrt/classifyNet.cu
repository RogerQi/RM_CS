#include "cudaUtility.h"
#include "opencv2/opencv.hpp"

__global__ void gpuImgCopy(cv::cuda::PtrStepSzb input, float* output, 
        uint32_t height, uint32_t width) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= width || y >= height)
        return ;

    output[y * 28 + x] = input(y, x);
}

cudaError_t cudaImgCopy(cv::cuda::GpuMat &input, float* output, 
        uint32_t height, uint32_t width) {
    const dim3 blockDim(8, 8);
    const dim3 gridDim(iDivUp(width, 8), iDivUp(height, 8));

    gpuImgCopy<<<gridDim, blockDim>>>(input, output, height, width);

    return CUDA(cudaGetLastError());
}
