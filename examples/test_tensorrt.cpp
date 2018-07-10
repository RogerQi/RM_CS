#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "opencv2/opencv.hpp"
#include "LeNet.h"

int main(void)
{
	/*
	 * create imageNet
	 */
	LeNet* net = LeNet::create("./model/lenet.prototxt",
            "./model/mnist_iter_75000.caffemodel");

	if( !net )
	{
		printf("Failed to init NN. Check model path\n");
		return 0;
	}

	float confidence = 0.0f;

    cv::cuda::GpuMat gmat1, gmat2;
    gmat1.upload(cv::imread("../../examples/four.jpg"));
    gmat2.upload(cv::imread("../../examples/four.jpg"));
    cv::cuda::cvtColor(gmat1, gmat1, cv::COLOR_BGR2GRAY);
    cv::cuda::cvtColor(gmat2, gmat2, cv::COLOR_BGR2GRAY);
    cv::cuda::resize(gmat1, gmat1, cv::Size(28, 28), 0, 0, cv::INTER_LINEAR);
    cv::cuda::resize(gmat2, gmat2, cv::Size(28, 28), 0, 0, cv::INTER_LINEAR);

    std::vector<cv::cuda::GpuMat> inputs = {gmat1, gmat2};

    std::vector<uint8_t> rs;
    std::vector<float>  conf;

    net->predict(inputs, &rs, &conf);

    printf("class: %u, confidence: %f\n", rs[0], conf[0]);
    printf("class: %u, confidence: %f\n", rs[1], conf[1]);

	return 0;
}
