#pragma once

#include "DetectorBase.h"
#include <ctime>
#include <cstdlib>

using namespace cv;
using namespace std;

class SmileDetector : public DetectorBase
{

private:
	const float max_smile_center_delta = 0.15;
	const Scalar smile_rect_color = Scalar(0, 255, 0);

	Mat gray_img;
	vector<Rect> head_rects;
	vector<Rect> smile_rects;
	CascadeClassifier face_cascade;
	CascadeClassifier smile_cascade;

	void process_low_accuracy(Mat& img)
	{
		SmileDetector::smile_cascade.detectMultiScale(SmileDetector::gray_img, SmileDetector::smile_rects);

		for (auto const& smileRect : SmileDetector::smile_rects)
		{
			rectangle(img, smileRect, SmileDetector::smile_rect_color);
		}
	}

	bool smile_is_valid(const Rect & smileRect)
	{
		for (const Rect & headRect : SmileDetector::head_rects)
		{
			// if smile is inside head
			if ((smileRect & headRect) == smileRect)
			{
				// calculate center
				Point smileCenter(smileRect.x + smileRect.width / 2, smileRect.y + smileRect.height / 2);
				Point headCenter(headRect.x + headRect.width / 2, headRect.y + headRect.height / 2);

				// smile is in the bottom part of the head & is cca in the middle
				if (smileCenter.y > headCenter.y && abs(smileCenter.x - headCenter.x) < headRect.width * SmileDetector::max_smile_center_delta)
				{
					return true;
				}
			}
		}
		return false;
	}

	void process_high_accuracy(Mat& img)
	{
		SmileDetector::face_cascade.detectMultiScale(gray_img, SmileDetector::head_rects);
		if (SmileDetector::head_rects.size() == 0)
		{
			// no head detected
			return;
		}

		SmileDetector::smile_cascade.detectMultiScale(gray_img, SmileDetector::smile_rects);
		for (const Rect & smileRect : SmileDetector::smile_rects)
		{
			if (SmileDetector::smile_is_valid(smileRect))
			{
				rectangle(img, smileRect, SmileDetector::smile_rect_color);
			}
		}
	}

public:
	bool highAccuracy;

	SmileDetector(bool highAccuracy)
	{
		if (!SmileDetector::face_cascade.load("haarcascades/haarcascade_frontalface_default.xml"))
			throw exception("ERROR: Could not load face cascade");

		if (!SmileDetector::smile_cascade.load("haarcascades/haarcascade_smile.xml"))
			throw exception("ERROR: Could not load eye cascade");

		SmileDetector::highAccuracy = highAccuracy;
	}

	SmileDetector() : SmileDetector(true)
	{
	}

	~SmileDetector()
	{
	}

	void Process(Mat& img)
	{
		cvtColor(img, SmileDetector::gray_img, COLOR_BGR2GRAY);

		if (SmileDetector::highAccuracy)
			SmileDetector::process_high_accuracy(img);
		else
			SmileDetector::process_low_accuracy(img);
	}
};

