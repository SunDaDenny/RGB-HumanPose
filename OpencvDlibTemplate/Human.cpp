#include "Human.h"

Human::Human(){

}

Human::~Human() {

}

void Human::init() {
	

}

void Human::updateObject(int index, std::vector<DetectObject>& stack) {
	
	int len = stack.size();
	int width = 0, height = 0;
	cv::Point center(0, 0);
	for (int i = 0; i < len; i++) {
		center.x += stack[i].getCenter().x;
		center.y += stack[i].getCenter().y;
		width += stack[i].getRect().width;
		height += stack[i].getRect().height;
	}
	center.x /= len;
	center.y /= len;
	width /= len;
	height /= len;

	cv::Rect rect( center.x - width * 0.5 , center.y - height * 0.5 , width , height );
	
	m_objects[index]->setExist(true);
	m_objects[index]->setRect(rect);

}

void Human::drawObjects(cv::Mat frame) {
	
	/*
	for (int i = 0; i < OBJECTS_NUM; i++) {
		if (m_objects[i]->isExist()) {
			
			cv::Scalar color;
			if (m_objects[i]->isLoss)color = cv::Scalar(0, 255, 0);
			else color = cv::Scalar(0, 0, 255);
			cv::rectangle(frame, m_objects[i]->getRect(), color, 5, 8);
		}	
	}
	*/

	int node_size = 20;
	int line_width = 3;

	//cv::rectangle(frame, m_objects[0]->getRect(), cv::Scalar(0, 255, 0) , 5, 8);
	if (m_objects[0]->isExist()) {
		cv::circle(frame, m_objects[0]->getCenter(), node_size, cv::Scalar(0, 255, 0), -1);
		cv::Point2f tmp(m_objects[0]->getCenter().x, frame.size().height );
		cv::line(frame, m_objects[0]->getCenter(), tmp, cv::Scalar(0, 255, 0), line_width);
	}

	if (m_objects[1]->isExist()) 
		cv::circle(frame, m_objects[1]->getCenter(), node_size, cv::Scalar(255, 0, 0), -1);
	if (m_objects[2]->isExist()) 
		cv::circle(frame, m_objects[2]->getCenter(), node_size, cv::Scalar(0, 0, 255), -1);
	if (m_objects[3]->isExist()) 
		cv::circle(frame, m_objects[3]->getCenter(), node_size, cv::Scalar(255, 255, 0), -1);
	if (m_objects[4]->isExist()) 
		cv::circle(frame, m_objects[4]->getCenter(), node_size, cv::Scalar(0, 255, 255), -1);
	

	if(m_objects[1]->isExist() && m_objects[3]->isExist())
		cv::line(frame, m_objects[1]->getCenter(), m_objects[3]->getCenter(), cv::Scalar(255, 0, 0), line_width);

	if(m_objects[2]->isExist() && m_objects[4]->isExist())
		cv::line(frame, m_objects[2]->getCenter(), m_objects[4]->getCenter(), cv::Scalar(0, 0, 255), line_width);


	cv::Point2f tmp(m_objects[0]->getCenter().x, m_objects[0]->getCenter().y + 100);
	if (m_objects[0]->isExist() && m_objects[3]->isExist()) {	
		cv::line(frame, tmp, m_objects[3]->getCenter(), cv::Scalar(255, 255, 0), line_width);
	}

	if (m_objects[0]->isExist() && m_objects[4]->isExist()) {
		cv::line(frame, tmp, m_objects[4]->getCenter(), cv::Scalar(0, 255, 255), line_width);
	}
	




}

