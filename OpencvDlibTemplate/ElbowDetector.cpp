#include "ElbowDetector.h"

ElbowDetector::ElbowDetector(std::string model_path) {

	m_model_path = model_path;
	init();

}

ElbowDetector::~ElbowDetector() {

}

void ElbowDetector::init() {

	std::ifstream fin( m_model_path, std::ios::binary);
	if (!fin) {
		printf("Error loading elbow hog\n");
	}
	deserialize(elbow_detector, fin);

}

DetectObject ElbowDetector::detect(cv::Mat frame, bool isRight) {

	int offset_x = isRight ? frame.cols * m_roi_offset_x : frame.cols * (1 - (m_roi_offset_x + m_roi_width));
	int offset_y = frame.rows * m_roi_offset_y;

	m_roi_rect = cv::Rect(  offset_x, offset_y,
							frame.cols * m_roi_width,
							frame.rows * m_roi_height);

	m_roi = frame(m_roi_rect);
	resize(m_roi, m_roi, cv::Size(m_roi.cols / m_roi_scale, m_roi.rows / m_roi_scale));

	dlib::cv_image<unsigned char> img(m_roi);

	std::vector<dlib::rectangle> elbows;
	elbows = elbow_detector(img);

	if (elbows.size() <= 0)
		return DetectObject(false);

	dlib::rectangle rect = elbows[0];

	cv::Rect result(rect.left() * m_roi_scale + offset_x,
					rect.top() * m_roi_scale + offset_y,
					(rect.right() - rect.left()) * m_roi_scale,
					(rect.bottom() - rect.top()) * m_roi_scale);

	return DetectObject(true, result);

}

void ElbowDetector::drawRoi(bool isRight) {

	std::string win;
	win = isRight ? "Right elbow" : "Left elbow";
	imshow(win, m_roi);

}

