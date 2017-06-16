#pragma once
#include <iostream>
#include <stdio.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/nonfree/features2d.hpp>  
#include <opencv2/core/core.hpp>  
#include < opencv2/opencv.hpp>  

#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>

class DetectObject
{	

	private :

		bool m_exist = true;

		cv::Rect m_rect;
		cv::Point m_center;

	public :

		bool isLoss = false;
		cv::Point2f last_gradient;

		DetectObject(bool);
		DetectObject(bool , cv::Rect);
		~DetectObject();

		cv::Point getCenter();
		void resizeRect(float scale);

		void setExist(bool b) { m_exist = b; };
		bool isExist() { return m_exist; };
		cv::Rect getRect() { return m_rect; };
		void setRect(cv::Rect rect) { m_rect = rect; };

};
