#pragma once

#include "DetectorBase.h"

using namespace cv;
using namespace std;

class EyeDetector : public DetectorBase
{

private:
	const Scalar headRectColor = Scalar(0, 0, 255);
	const Scalar eyeRectColor = Scalar(0, 255, 0);

	Mat grayImg;
	vector<Rect> headRects;
	vector<Rect> eyeRects;

	CascadeClassifier face_cascade;
	CascadeClassifier eye_cascade;
	
public:
	EyeDetector()
	{
		if (!face_cascade.load("haarcascades/haarcascade_frontalface_default.xml"))
		{
			throw exception("ERROR: Could not load face cascade");
		}		
		if (!eye_cascade.load("haarcascades/haarcascade_eye.xml"))
		{
			throw exception("ERROR: Could not load eye cascade");
		}
	}

	~EyeDetector()
	{
	}

	void Process(Mat& img)
	{
		cvtColor(img, grayImg, COLOR_BGR2GRAY);

		eye_cascade.detectMultiScale(grayImg, eyeRects);

		if (eyeRects.size() > 0)
			eye_cascade.detectMultiScale(grayImg, headRects);

		for (auto const& eyeRect : eyeRects)
		{
			for (auto const& headRect : headRects)
			{
				// if smile is inside head
				if ((eyeRect & headRect) == eyeRect)
				{
					rectangle(img, eyeRect, eyeRectColor);
				}
			}
		}
	}
};

