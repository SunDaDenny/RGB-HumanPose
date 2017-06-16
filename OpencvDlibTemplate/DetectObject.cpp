#include "DetectObject.h"

DetectObject::DetectObject(bool exist) {
	m_exist = exist;
}

DetectObject::DetectObject(bool exist , cv::Rect rect) {
	m_exist = exist;
	m_rect = rect;
}

DetectObject::~DetectObject() {

}

cv::Point DetectObject::getCenter() {

	return cv::Point( m_rect.x + m_rect.width*0.5, m_rect.y + m_rect.height*0.5);

}

void DetectObject::resizeRect(float scale) {
	
	m_rect.x += m_rect.width * (1 - scale) / 2;
	m_rect.y += m_rect.height * (1 - scale) / 2;
	m_rect.width *= scale;
	m_rect.height *= scale;

}