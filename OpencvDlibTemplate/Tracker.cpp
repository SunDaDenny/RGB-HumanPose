#include "Tracker.h"

Tracker::Tracker() {
	
}

Tracker::~Tracker() {

}

void Tracker::init(cv::Mat frame) {

	prev_im = frame.clone();
	is_init = true;

}

void Tracker::update(cv::Mat frame) {

	if (frame.empty())
		return;

	if (!is_init)
		init(frame);

	nex_im = frame.clone();
	resize(nex_im, nex_im, prev_im.size());

	if (!prev_im.empty() && !nex_im.empty()){
		cv::calcOpticalFlowFarneback(prev_im, nex_im, flow, 0.5, 1, 12, 2, 7, 1.5, 0);
	}
	
	prev_im = nex_im.clone();

}

void Tracker::drawOptFlowMap(cv::Mat frame) {

	int step = 10;
	cv::Scalar color( 0, 255 ,0 );

	for (int y = 0; y < frame.rows; y += step)
		for (int x = 0; x < frame.cols; x += step)
		{
			cv::Point2f& fxy = flow.at< cv::Point2f>(y, x);
			cv::line(frame, cv::Point(x, y), cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)),
				color);
			circle(frame, cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)), 1, color, -1);
		}

}

cv::Rect Tracker::updateRect(cv::Rect rect) {
	
	cv::Point2f gradient( 0.0 , 0.0 );

	for (int y = rect.y ; y < rect.y + rect.height; y++ )
		for (int x = rect.x; x < rect.x + rect.width; x++) {
			gradient += flow.at< cv::Point2f >(y, x);
		}

	float eta = 1.0f;
	gradient.x /= ( rect.width * rect.height );
	gradient.y /= ( rect.width * rect.height );
	gradient *= eta;

	cv::Rect ans( cvRound( rect.x + gradient.x ) , cvRound(rect.y + gradient.y) , rect.width , rect.height);
	return ans;

}


void Tracker::updateHuman(Human human) {

	for (int i = 0; i < OBJECTS_NUM; i++) {

		if (human.m_objects[i]->isExist()) {
			human.m_objects[i]->setRect(updateRect(human.m_objects[i]->getRect()));
		}

	}

}

