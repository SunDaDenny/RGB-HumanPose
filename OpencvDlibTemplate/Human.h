#pragma once

#include "DetectObject.h"

#define OBJECTS_NUM 5

class Human 
{

	private:

	public:

		DetectObject *m_objects[OBJECTS_NUM] = {
			new DetectObject(false) ,
			new DetectObject(false) ,
			new DetectObject(false) ,
			new DetectObject(false) ,
			new DetectObject(false)
		};

		bool is_ready[OBJECTS_NUM] = {false};

		Human();
		~Human();
		
		void init();
		void updateObject(int, std::vector<DetectObject>&);

		void drawObjects(cv::Mat);

};