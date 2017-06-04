#pragma once

#include "DetectorBase.h"
#include <ctime>
#include <cstdlib>

using namespace cv;
using namespace std;

class SmileDetector : public DetectorBase
{

private:
	const Scalar headRectColor = Scalar(0, 0, 255);
	const Scalar smileRectColor = Scalar(0, 255, 0);

	Mat grayImg;
	vector<Rect> headRects;
	vector<Rect> smileRects;

	CascadeClassifier face_cascade;
	CascadeClassifier smile_cascade;

public:
	bool showHeadRect;

	SmileDetector(bool showHeadRect)
	{
		if (!face_cascade.load("haarcascades/haarcascade_frontalface_default.xml"))
		{
			throw exception("ERROR: Could not load face cascade");
		}
		if (!smile_cascade.load("haarcascades/haarcascade_smile.xml"))
		{
			throw exception("ERROR: Could not load eye cascade");
		}

		SmileDetector::showHeadRect = showHeadRect;
	}

	SmileDetector() : SmileDetector(false)
	{
	}

	~SmileDetector()
	{
	}

	void Process(Mat& img)
	{
		cvtColor(img, grayImg, COLOR_BGR2GRAY);

		smile_cascade.detectMultiScale(grayImg, smileRects);

		for (auto const& smileRect : smileRects)
		{
			rectangle(img, smileRect, smileRectColor);
		}
	}

	// vylepsene - skenuje aj tvar
	void Process2(Mat& img)
	{
		cvtColor(img, grayImg, COLOR_BGR2GRAY);

		smile_cascade.detectMultiScale(grayImg, smileRects);
		
		if (smileRects.size() > 0)
			face_cascade.detectMultiScale(grayImg, headRects);

		for (auto const& smileRect : smileRects)
		{
			for (auto const& headRect : headRects)
			{
				// if smile is inside head
				if ((smileRect & headRect) == smileRect)
				{
					// calculate center
					Point smileCenter(smileRect.x + smileRect.width / 2, smileRect.y + smileRect.height / 2);
					Point headCenter(headRect.x + headRect.width / 2, headRect.y + headRect.height / 2);

					cout << smileCenter.x << " " << smileCenter.y << ", " << headCenter.x << " " << headCenter.y << " abs: " << abs(smileCenter.x - headCenter.x) << " q:" << headRect.width / 4 << endl;

					// smile is always in the bottom part of the head
					if (smileCenter.y > headCenter.y && abs(smileCenter.x - headCenter.x) < headRect.width / 4);
					{
						rectangle(img, smileRect, smileRectColor);

						if (showHeadRect)
							rectangle(img, headRect, headRectColor);
					}
				}
			}
		}
	}
};

