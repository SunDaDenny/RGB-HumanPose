#pragma once
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <thread>
#include <chrono>

#include "DetectObject.h"
#include "HeadDetector.h"
#include "HandDetector.h"
#include "ElbowDetector.h"
#include "Human.h"
#include "Tracker.h"
#include "PLKTracker.h"
#include "TrackerGPU.h"

#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/opencv.h>

void init();
void initDetectThreads();

void detectHeads(cv::Mat frame);
void drawHeads(cv::Mat frame);

void detectHands(cv::Mat frame);
void drawHands(cv::Mat frame);

void detectElbows(cv::Mat frame);
void drawElbows(cv::Mat frame);

void checkDetection(cv::Mat frame);
void changeStage(int stage);

void drawEllipse(cv::Mat frame, cv::Rect rect, cv::Scalar color);

void detectThreadTask();

Human *m_human;
Tracker *m_tracker;
PLKTracker *m_plktracker;
TrackerGPU *m_trackergpu;

HeadDetector *head_detector;
std::vector<DetectObject> heads_stack;

HandDetector *hand_detector;
std::vector<DetectObject> hands_right_stack;
std::vector<DetectObject> hands_left_stack;

ElbowDetector *elbow_right_detector;
std::vector<DetectObject> elbows_right_stack;
ElbowDetector *elbow_left_detector;
std::vector<DetectObject> elbows_left_stack;
