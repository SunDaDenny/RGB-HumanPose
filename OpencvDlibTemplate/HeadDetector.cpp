#include "HeadDetector.h"

HeadDetector::HeadDetector(std::string model_path) {

	m_model_path = model_path;
	init();

}

HeadDetector::~HeadDetector() {

}

void HeadDetector::init() {
	
	face_detector = dlib::get_frontal_face_detector();

}

DetectObject HeadDetector::detect(cv::Mat frame) {

	int offset_x = frame.cols * m_roi_offset_x;
	int offset_y = frame.rows * m_roi_offset_y;

	m_roi_rect = cv::Rect(	offset_x, offset_y,
							frame.cols * m_roi_width, 
							frame.rows * m_roi_height );

	m_roi = frame(m_roi_rect);
	resize(m_roi, m_roi, cv::Size(m_roi.cols / m_roi_scale, m_roi.rows / m_roi_scale));

	dlib::cv_image<dlib::bgr_pixel> cimg(m_roi);
	std::vector<dlib::rectangle> faces = face_detector(cimg);

	if (faces.size() <= 0) 	
		return  DetectObject(false);
	
	dlib::rectangle rect = faces[0];

	cv::Rect result( rect.left() * m_roi_scale + offset_x,
					 rect.top() * m_roi_scale + offset_y,
					(rect.right() - rect.left()) * m_roi_scale,
					(rect.bottom() - rect.top()) * m_roi_scale );

	return DetectObject(true, result);

}

void HeadDetector::drawRoi() {

	imshow("Head", m_roi);

}