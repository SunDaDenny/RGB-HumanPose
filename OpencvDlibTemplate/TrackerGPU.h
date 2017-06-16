#pragma once

#include "DetectObject.h"
#include "Human.h"
#include "OpticalFlow_GPU.h"

class TrackerGPU {

private:

	cv::Mat prev_im;
	cv::Mat nex_im;
	cv::Mat flow;
	cv::Mat dense;

	cv::Point2f vector[640][480];

	OpticalFlow_GPU opticalFlow;

public:

	bool is_init = false;

	TrackerGPU();
	~TrackerGPU();

	void init(cv::Mat);
	void update(cv::Mat);
	void drawOptFlowMap(cv::Mat);

	cv::Rect updateRect(cv::Rect);
	void updateHuman(Human);

	cv::Mat getFlowDense() { return dense; }

};