#pragma once

#include "DetectObject.h"
#include "Human.h"
#include "OpticalFlow_GPU.h"

class PLKTracker {

private:

	cv::Mat prev_im;
	cv::Mat nex_im;
	std::vector<cv::Point2f> prev_feat;
	std::vector<cv::Point2f> nex_feat;
	std::vector<uchar> found_feat;
	cv::Mat err;

	cv::Mat flow_dense;
	cv::Mat color_dense;

	cv::Point2f last_gradient;
	bool isLoss = false;

public:

	bool is_init = false;

	PLKTracker();
	~PLKTracker();

	void init(cv::Mat);
	void update(cv::Mat ,cv::Mat);

	void drawFlow(cv::Mat);
	void drawFeature(cv::Mat);

	cv::Point2f getGradient(cv::Rect);
	cv::Point2f getEstimateGradient(cv::Rect);

	cv::Rect updateRect(cv::Rect);
	void updateHuman(Human);

	void updateDense(cv::Mat);
	float checkDenseInRoi(cv::Rect);

	float checkColorInRoi(cv::Rect);

};
