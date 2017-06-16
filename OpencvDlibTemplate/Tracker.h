#pragma once

#include "DetectObject.h"
#include "Human.h"

class Tracker {

	private :

		cv::Mat prev_im;
		cv::Mat nex_im;
		cv::Mat flow;

	public :

		bool is_init = false;

		Tracker();
		~Tracker();

		void init(cv::Mat);
		void update(cv::Mat);
		void drawOptFlowMap( cv::Mat);

		cv::Rect updateRect(cv::Rect);
		void updateHuman(Human);
};
