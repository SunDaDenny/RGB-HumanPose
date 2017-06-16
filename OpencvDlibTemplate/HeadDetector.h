#pragma once

#include "DetectObject.h"

class HeadDetector
{

	private:
		std::string m_model_path;

		cv::Mat m_roi;
		cv::Rect m_roi_rect;
		float m_roi_width = 1.0, m_roi_height = 0.5;
		float m_roi_offset_x = 0.0, m_roi_offset_y = 0;
		float m_roi_scale = 1.0;

		dlib::frontal_face_detector face_detector;

	public:

		HeadDetector(std::string model_path);
		~HeadDetector();
		
		void init();
		DetectObject detect(cv::Mat frame);
		void drawRoi();
};