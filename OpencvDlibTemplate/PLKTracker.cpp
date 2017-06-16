#include "PLKTracker.h"

PLKTracker::PLKTracker() {

}

PLKTracker::~PLKTracker() {

}

void PLKTracker::init(cv::Mat frame) {

	prev_im = frame.clone();
	is_init = true;
}

void PLKTracker::update(cv::Mat frame , cv::Mat color) {

	if (frame.empty())
		return;

	if (!is_init)
		init(frame);

	cv::Mat HSV;
	cv::cvtColor(color, HSV , CV_BGR2HSV);
	cv::inRange(HSV, cv::Scalar(0, 58, 40), cv::Scalar(35, 174, 255), color_dense);
	imshow("thr", color_dense);

	nex_im = frame.clone();
	resize(nex_im, nex_im, prev_im.size());

	int num_feat_sample = 1000;
	prev_feat.resize(num_feat_sample);
	cv::goodFeaturesToTrack(prev_im, prev_feat, num_feat_sample, 0.01, 0.1, cv::Mat(), 3 , false , 0.04);

	int num_rand_sample = 2000;
	prev_feat.resize( prev_feat.size() + num_rand_sample );
	for (int i = 0; i < num_rand_sample; i++) {
		cv::Point2f p_rand( rand() % frame.size().width, rand() % frame.size().height);
		prev_feat.push_back(p_rand);
	}

	if (!prev_im.empty() && !nex_im.empty()) {
		cv::calcOpticalFlowPyrLK(prev_im, nex_im, prev_feat, nex_feat, found_feat, err );
	}

	prev_im = nex_im.clone();
	
}

cv::Point2f PLKTracker::getGradient(cv::Rect rect) {

	cv::Point2f gradient(0.0, 0.0);
	int count = 0;

	for (int i = 0; i < prev_feat.size(); i++) {
		if (found_feat[i]) {
			if (rect.contains(prev_feat[i])) {		
				cv::Point2f vector = nex_feat[i] - prev_feat[i];
				float dis = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
				gradient += (nex_feat[i] - prev_feat[i]);
				count++;				
			}
		}
	}
	
	if(count == 0)
		return gradient;

	gradient.x /= count;
	gradient.y /= count;

	return gradient;
}

float angleBetween(const cv::Point2f &v1, const cv::Point2f &v2)
{
	float len1 = sqrt(v1.x * v1.x + v1.y * v1.y);
	float len2 = sqrt(v2.x * v2.x + v2.y * v2.y);

	float dot = v1.x * v2.x + v1.y * v2.y;

	float a = dot / (len1 * len2);

	if (a >= 1.0)
		return 0.0;
	else if (a <= -1.0)
		return 3.1415926;
	else
		return acos(a); // 0..PI
}

cv::Point2f PLKTracker::getEstimateGradient(cv::Rect rect) {

	float maxScore = 0;

	cv::Point2f gradient_orig = getGradient(rect);
	cv::Point2f gradient( 0, 0 );

	int region = 20;

	for (int i = -region; i <= region ; i++) {
		for (int j = -region; j <= region; j++) {

			cv::Rect check( cvRound(rect.x + i), cvRound(rect.y + j), rect.width, rect.height);
			float p = checkDenseInRoi(check);
			//cv::Point2f g = getGradient(check);

			cv::Point2f vec(i ,j);
			int dis =  sqrt( pow( vec.x - gradient_orig.x , 2 ) + pow(vec.y - gradient_orig.y, 2) );
			//int dis = sqrt(pow(vec.x - g.x, 2) + pow(vec.y - g.y, 2));
			
			float angle = angleBetween(gradient_orig , vec);
			float mask = (p + 0.25 > 1.0) ? 1.0 : p + 0.25;

			float color = checkColorInRoi(rect);
			
			//float score = (float)(region - dis) * p;
			//float score = (float)p;
			float score = (180 - angle) * p;

			if (score > maxScore) {				
				maxScore = score;
				gradient =  vec;
			}
		}
	}

	/*
	int step = 10;
	int stride_x = ( rect.width * 3 ) / step;
	int stride_y = ( rect.height * 3 ) / step;

	for (int i = 0 ; i < step ; i ++ ) {
		for (int j = 0 ; j < step ; j ++) {

			cv::Rect check( cvRound( rect.x - rect.width + stride_x * i ), 
							cvRound( rect.y - rect.height + stride_y * j )
							, stride_x , stride_y);

			float p = checkDenseInRoi(check);

			cv::Point2f vec( (check.x - rect.x) , (check.y - rect.y) );
	
			int score = ( 180 - acos(vec.dot(gradient_orig))) - ( (pow(vec.x, 2) + pow(vec.y, 2)) ) * 0.5;

			if (score > maxScore) {
				maxScore = score;
				gradient = vec;
				//printf("  %f %f\n", vec.x, vec.y);
			}

		}
	}
	*/

	return gradient;
}

cv::Rect PLKTracker::updateRect(cv::Rect rect) {
	
	//cv::Point2f gradient = getGradient(rect);
	cv::Point2f gradient = getEstimateGradient(rect);

	cv::Rect ans(cvRound(rect.x + gradient.x), cvRound(rect.y + gradient.y), rect.width, rect.height);

	return ans;
	
}


void  PLKTracker::updateHuman(Human human) {

	for (int i = 0; i < OBJECTS_NUM; i++) {

		if (i == 3 || i == 4)
			continue;

		if (human.m_objects[i]->isExist()) {
			
			cv::Rect rect = (human.m_objects[i]->getRect());
			float p = checkDenseInRoi(rect);
			
			cv::Point2f sp = getGradient(rect);
			float speed = sqrtf(pow(sp.x, 2) + pow(sp.y, 2));

			/*
			cv::Point2f g = getEstimateGradient(human.m_objects[i]->getRect());
			cv::Rect newRect(cvRound(rect.x + g.x), cvRound(rect.y + g.y), rect.width, rect.height);
			human.m_objects[i]->setRect(newRect);
			human.m_objects[i]->isLoss = true;
			*/

			human.m_objects[i]->setRect(updateRect(rect));
			human.m_objects[i]->isLoss = false;
			
		}
	}

}

void PLKTracker::updateDense(cv::Mat dense) {

	flow_dense = dense;
}

float PLKTracker::checkDenseInRoi(cv::Rect roi) {
	
	if (flow_dense.empty()) {
		return 0.0f;
	}

	if (roi.x < 0)roi.x = 0;
	if (roi.y < 0)roi.y = 0;
	if (roi.x + roi.width > 640)roi.x = 640 - roi.width;
	if (roi.y + roi.height > 480)roi.y = 480 - roi.height;

	cv::Mat flow_roi = flow_dense(roi);
	//cv::imshow("WW", flow_roi);

	int n = cv::countNonZero(flow_roi);
	float ans = (float)n / (roi.width * roi.height);

	return ans;
}


float PLKTracker::checkColorInRoi(cv::Rect roi) {
	
	if (color_dense.empty()) {
		return 0.0f;
	}

	if (roi.x < 0)roi.x = 0;
	if (roi.y < 0)roi.y = 0;
	if (roi.x + roi.width > 640)roi.x = 640 - roi.width;
	if (roi.y + roi.height > 480)roi.y = 480 - roi.height;

	cv::Mat color_roi = color_dense(roi);

	int n = cv::countNonZero(color_roi);
	float ans = (float)n / (roi.width * roi.height);

	return ans;
}

void PLKTracker::drawFlow(cv::Mat frame) {
	
	for (int i = 0; i < prev_feat.size(); i++) {
		if ( found_feat[i] ) {
			cv::line( frame, prev_feat[i], nex_feat[i], cv::Scalar(0, 0, 255));
		}
	}

	//cv::imshow("GPU DENSE", flow_dense);
}

void PLKTracker::drawFeature(cv::Mat frame) {

	for (int i = 0; i < nex_feat.size(); i++)
		//if (!flow_dense.empty() && flow_dense.at<uchar>(cv::Point(nex_feat[i].x, nex_feat[i].y)) > 0)
			circle(frame, cv::Point( cvRound( nex_feat[i].x ), cvRound( nex_feat[i].y ) ), 2, cv::Scalar(0, 0, 255), -1);

}
