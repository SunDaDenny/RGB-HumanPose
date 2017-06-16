#include "TrackerGPU.h"

TrackerGPU::TrackerGPU() {

	if (!opticalFlow.initialize(640, 480, "OpenGL_Shader" , 10 , 2.5))
	{
		cout << "Fail to initialize Optical Flow class." << endl;
	}
}

TrackerGPU::~TrackerGPU() {

}

void TrackerGPU::init(cv::Mat frame) {


}

void TrackerGPU::update(cv::Mat frame) {

	if (frame.empty())
		return;

	if (prev_im.empty()) {
		prev_im = frame.clone();
	}
		
	opticalFlow.run(prev_im, frame);

	dense = opticalFlow.getMaskMap();
	//imshow("GPU Color Vector", opticalFlow.getColorVectorMap());
	//imshow("GPU Mask", dense);

	prev_im = frame.clone();

}

void TrackerGPU::drawOptFlowMap(cv::Mat frame) {

	int step = 10;
	cv::Scalar color(0, 255, 0);

	cv::Mat vecmap = opticalFlow.getVectorData();

	for (int y = 0; y < frame.rows; y += step)
		for (int x = 0; x < frame.cols; x += step)
		{
			cv::Point2f fxy(vecmap.at<cv::Vec3f>(y, x)[0] , vecmap.at<cv::Vec3f>(y, x)[1]);
			cv::line(frame, cv::Point(x, y), cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)),
				color);
			circle(frame, cv::Point(cvRound(x + fxy.x), cvRound(y + fxy.y)), 1, color, -1);
		}

}

cv::Rect TrackerGPU::updateRect(cv::Rect rect) {

	cv::Point2f gradient(0.0, 0.0);
	cv::Mat vecmap = opticalFlow.getVectorData();

	for (int y = rect.y; y < rect.y + rect.height; y++)
		for (int x = rect.x; x < rect.x + rect.width; x++) {
			cv::Point2f fxy(vecmap.at<cv::Vec3f>(y, x)[0], vecmap.at<cv::Vec3f>(y, x)[1]);
			gradient += fxy;
		}

	float eta = 1.0f;
	gradient.x /= (rect.width * rect.height);
	gradient.y /= (rect.width * rect.height);
	gradient *= eta;

	cv::Rect ans(cvRound(rect.x + gradient.x), cvRound(rect.y + gradient.y), rect.width, rect.height);
	return ans;

}


void TrackerGPU::updateHuman(Human human) {

	for (int i = 0; i < OBJECTS_NUM; i++) {

		if (human.m_objects[i]->isExist()) {
			human.m_objects[i]->setRect(updateRect(human.m_objects[i]->getRect()));
		}

	}

}
