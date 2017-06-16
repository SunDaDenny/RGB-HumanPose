#include "main.h"

#define HANDS_STACK_MAX 5
#define HEADS_STACK_MAX 5
#define ELBOWS_STACK_MAX 5

using namespace std;
using namespace cv;
using namespace dlib;

clock_t begin_time;

int cur_stage = 0;

string window_name = "HumanPose";

Mat gobal_frame;
Mat gobal_gray_frame;

bool isCamera = false;

int main(int argc, const char** argv)
{
	CvCapture* capture;

	VideoCapture cap("data/Videoclip5.wmv");
	if (!isCamera) {
		if (!cap.isOpened()) {
			cout << "Cannot open the video file on C++ API" << endl;
			return -1;
		}
		else {
			cap.set(CV_CAP_PROP_FPS, 30);
		}
	}

	Mat frame, frame_gray;

	init();
	
	capture = cvCaptureFromCAM(0);

	if (capture) {

		while (true) {

			begin_time = clock();

			frame = cvQueryFrame(capture);			
			if(!isCamera)cap.read(frame);	

			gobal_frame = frame.clone();
			
			if (!frame.empty()) {

				cvtColor(frame, frame_gray, CV_BGR2GRAY);
				gobal_gray_frame = frame_gray.clone();

				if (cur_stage == 0) {

					detectHeads(frame);
					detectHands(frame_gray);
					detectElbows(frame_gray);

					drawHeads(frame);
					drawHands(frame);
					drawElbows(frame);

					m_human->drawObjects(frame);

					checkDetection(frame);

				}
				else if (cur_stage == 1) {
							
					//m_tracker->update(frame_gray);
					//m_tracker->updateHuman( *m_human );					
					//m_human->drawObjects(frame);
					//m_tracker->drawOptFlowMap(frame);

					m_trackergpu->update(frame);
					m_plktracker->update(frame_gray , frame);
					m_plktracker->updateDense(m_trackergpu->getFlowDense());
					m_plktracker->updateHuman(*m_human);	

					m_human->drawObjects(frame);
					//m_plktracker->drawFeature(frame);
					//m_plktracker->drawFlow(frame);	

					//m_trackergpu->updateHuman(*m_human);
					//m_human->drawObjects(frame);
					//m_trackergpu->drawOptFlowMap(frame);

					Mat tmp = m_trackergpu->getFlowDense().clone();
					cvtColor(tmp, tmp, CV_GRAY2BGR);
					m_human->drawObjects(tmp);
					imshow("Dense", tmp);

				}

				imshow(window_name, frame);

			}
			else {
				printf("No capture frame\n");
				//break;
			}

			int key = cv::waitKey(30) & 255;
			if (key == 27) {
				cvReleaseCapture(&capture);
				break;
			}
			else if (key == 'a') {
				cur_stage = 1;
			}

			//printf("FPS : %f\n", 1000 / float(clock() - begin_time));

		}
	}
	return 0;
}

void detectHeads(Mat frame) {

	DetectObject head = head_detector->detect(frame);
	head_detector->drawRoi();

	if (head.isExist()) {

		if (heads_stack.size() >= HEADS_STACK_MAX)
			heads_stack.erase(heads_stack.begin());
		heads_stack.push_back(head);

	}

}

void detectHands(Mat frame) {

	DetectObject right_hand = hand_detector->detect(frame , true);
	hand_detector->drawRoi(true);

	if (right_hand.isExist()) {

		right_hand.resizeRect(0.5);
		if ( hands_right_stack.size() >= HANDS_STACK_MAX)
			hands_right_stack.erase(hands_right_stack.begin());
		hands_right_stack.push_back(right_hand);
	}

	DetectObject left_hand = hand_detector->detect(frame, false);
	hand_detector->drawRoi(false);
	
	if (left_hand.isExist()) {

		left_hand.resizeRect(0.5);
		if (hands_left_stack.size() >= HANDS_STACK_MAX)
			hands_left_stack.erase(hands_left_stack.begin());
		hands_left_stack.push_back(left_hand);
	}

}

void detectElbows(Mat frame) {

	DetectObject right_elbow = elbow_right_detector->detect(frame, true);
	elbow_right_detector->drawRoi(true);

	if (right_elbow.isExist()) {

		right_elbow.resizeRect(0.5);
		if (elbows_right_stack.size() >= ELBOWS_STACK_MAX)
			elbows_right_stack.erase(elbows_right_stack.begin());
		elbows_right_stack.push_back(right_elbow);
	}

	DetectObject left_elbow = elbow_left_detector->detect(frame, false);
	elbow_left_detector->drawRoi(false);

	if (left_elbow.isExist()) {

		left_elbow.resizeRect(0.5);
		if (elbows_left_stack.size() >= ELBOWS_STACK_MAX) 
			elbows_left_stack.erase(elbows_left_stack.begin());
		elbows_left_stack.push_back(left_elbow);
	}

}

void detectThreadTask() {

	while (true) {

		if (gobal_frame.empty()) printf("empty frame\n");
		else {
			DetectObject head = head_detector->detect(gobal_frame);
			if (head.isExist()) {
				printf("Got Face\n");
				m_human->m_objects[0]->setRect(head.getRect());
			}
			DetectObject right_hand = hand_detector->detect(gobal_gray_frame, true);
			if (right_hand.isExist()) {
				right_hand.resizeRect(0.5);
				printf("Got right_hand\n");
				m_human->m_objects[1]->setRect(right_hand.getRect());
			}
			DetectObject left_hand = hand_detector->detect(gobal_gray_frame, false);
			if (left_hand.isExist()) {
				left_hand.resizeRect(0.5);
				printf("Got left_hand\n");
				m_human->m_objects[2]->setRect(left_hand.getRect());
			}
			DetectObject right_elbow = elbow_right_detector->detect(gobal_gray_frame, true);
			if (right_elbow.isExist()) {
				printf("Got right_elbow\n");
				right_elbow.resizeRect(0.5);
				m_human->m_objects[3]->setRect(right_elbow.getRect());
			}
			DetectObject left_elbow = elbow_left_detector->detect(gobal_gray_frame, false);
			if (left_elbow.isExist()) {
				printf("Got left_elbow\n");
				left_elbow.resizeRect(0.5);
				m_human->m_objects[4]->setRect(left_elbow.getRect());
			}
		}

		chrono::duration<int> delay(1);
		//this_thread::sleep_for(delay);
	}

	return;
}

void checkDetection(Mat frame) {

	if (heads_stack.size() >= HEADS_STACK_MAX) {
		m_human->updateObject(0, heads_stack);
		m_human->is_ready[0] = true;
	}
	if (hands_right_stack.size() >= HANDS_STACK_MAX) {
		m_human->updateObject(1, hands_right_stack);
		m_human->is_ready[1] = true;
	}
	if (hands_left_stack.size() >= HANDS_STACK_MAX) {
		m_human->updateObject(2, hands_left_stack);
		m_human->is_ready[2] = true;
	}
	if (elbows_right_stack.size() >= ELBOWS_STACK_MAX) {
		m_human->updateObject(3, elbows_right_stack);
		m_human->is_ready[3] = true;
	}
	if (elbows_left_stack.size() >= ELBOWS_STACK_MAX) {
		m_human->updateObject(4, elbows_left_stack);
		m_human->is_ready[4] = true;
	}

	
	if( m_human->is_ready[0] && m_human->is_ready[1] && m_human->is_ready[2] )
		changeStage(1);

	/*
	for (int i = 0; i < 5; i++)
		if (!m_human->is_ready[i])break;
		else if (i == 4)changeStage(1);
	*/

	/*
	for (int i = 0; i < 5; i++)
		printf("%d ", m_human->is_ready[i]);
	printf("\n");
	*/
}

void changeStage(int stage) {

	cur_stage = stage;

	if (stage == 1) {
		initDetectThreads();
	}

}

void drawHeads(Mat frame) {

	for (int i = 0; i < heads_stack.size(); i++) {

		drawEllipse(frame, heads_stack[i].getRect(), Scalar(0, 255, 0));

	}

}

void drawHands(Mat frame) {

	for (int i = 0; i < hands_right_stack.size(); i++) {
		drawEllipse(frame, hands_right_stack[i].getRect() , Scalar(0, 0, 255));
	}

	for (int i = 0; i < hands_left_stack.size(); i++) {
		drawEllipse(frame, hands_left_stack[i].getRect(), Scalar(0, 0, 255));
	}

}

void drawElbows(Mat frame) {

	for (int i = 0; i < elbows_right_stack.size(); i++) {
		drawEllipse(frame, elbows_right_stack[i].getRect(), Scalar(255, 0, 0));
	}

	for (int i = 0; i < elbows_left_stack.size(); i++) {
		drawEllipse(frame, elbows_left_stack[i].getRect(), Scalar(255, 0, 0));
	}

}

void init() {

	m_human = new Human();

	head_detector = new HeadDetector("None");
	hand_detector = new HandDetector("data/hand3.xml");
	elbow_right_detector = new ElbowDetector("data/left_elbow_S001.svm");
	elbow_left_detector = new ElbowDetector("data/right_elbow_S001.svm");
	
	heads_stack.clear();
	hands_right_stack.clear();
	hands_left_stack.clear();
	elbows_right_stack.clear();
	elbows_left_stack.clear();
	
	m_tracker = new Tracker();
	m_plktracker = new PLKTracker();
	m_trackergpu = new TrackerGPU();
}

void initDetectThreads() {

	thread threadDetectHeads(detectThreadTask);
	threadDetectHeads.detach();

	return;
}

void drawEllipse(Mat frame, Rect rect, Scalar color) {

	Point center(rect.x + rect.width*0.5, rect.y + rect.height*0.5);
	ellipse(frame, center, Size(rect.width*0.5, rect.height*0.5), 0, 0, 360, color, 4, 8, 0);

}