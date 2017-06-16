#pragma once

#include "DetectObject.h"

class HandDetector
{
	
	private:
		std::string m_model_path;

		cv::Mat m_roi;
		cv::Rect m_roi_rect;
		float m_roi_width = 0.5, m_roi_height = 0.5;
		float m_roi_offset_x = 0.0, m_roi_offset_y = 0.0;
		float m_roi_scale = 1.0;
		
		cv::CascadeClassifier hand_detector;

	public:

		HandDetector(std::string model_path);
		~HandDetector();

		void init();
		DetectObject detect(cv::Mat frame , bool isRight);
		void drawRoi(bool isRight);
};



