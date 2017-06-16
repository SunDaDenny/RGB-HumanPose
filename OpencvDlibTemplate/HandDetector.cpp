#include "HandDetector.h"

HandDetector::HandDetector(std::string model_path) {

	m_model_path = model_path;
	init();

}

HandDetector::~HandDetector() {

}

void HandDetector::init() {

	if (!hand_detector.load(m_model_path)) {
		printf("Error loading hand hog\n");  
	};

}

DetectObject HandDetector::detect(cv::Mat frame , bool isRight) {

	int offset_x = isRight ? frame.cols * m_roi_offset_x : frame.cols * ( 1 - (m_roi_offset_x + m_roi_width));
	int offset_y = frame.rows * m_roi_offset_y;

	m_roi_rect = cv::Rect(	offset_x, offset_y,
							frame.cols * m_roi_width,
							frame.rows * m_roi_height);

	m_roi = frame(m_roi_rect);
	resize(m_roi, m_roi, cv::Size(m_roi.cols / m_roi_scale, m_roi.rows / m_roi_scale));
	
	cv::Mat frame_equ;
	equalizeHist(m_roi, frame_equ);

	std::vector<cv::Rect> hands;
	hand_detector.detectMultiScale(frame_equ, hands , 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(80, 80));

	if (hands.size() <= 0)
		return  DetectObject(false);

	cv::Rect rect = hands[0];

	cv::Rect result( rect.x * m_roi_scale + offset_x,
					 rect.y * m_roi_scale + offset_y,
					 rect.width * m_roi_scale,
					 rect.height * m_roi_scale );

	return DetectObject(true, result);

}

void HandDetector::drawRoi(bool isRight) {

	std::string win;
	win = isRight ? "Right hand" : "Left hand";
	imshow(win, m_roi);

}