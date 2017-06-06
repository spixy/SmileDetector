#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class SmileDetector
{
private:
	const double max_smile_center_delta = 0.5;
	const double min_smile_center_y_percentage = 0.7;

	vector<Rect> head_rects;
	vector<Rect> smile_rects;
	CascadeClassifier face_cascade;
	CascadeClassifier smile_cascade;
	VideoCapture video_capture;

	bool smile_is_valid(const Rect & smileRect)
	{
		for (const Rect & headRect : SmileDetector::head_rects)
		{
			int headRectHalfWidth = headRect.width / 2;
			int headRectHalfHeight = headRect.height / 2;

			// if smile is inside head
			if (smileRect.x > headRect.x &&
				smileRect.x < headRect.x + headRectHalfWidth &&
				smileRect.x + smileRect.width > headRect.x + headRectHalfWidth &&
				smileRect.x + smileRect.width < headRect.x + headRect.width &&
				smileRect.y > headRect.y &&
				smileRect.y + smileRect.height < headRect.y + headRect.height &&
				smileRect.y + smileRect.height / 2 > headRect.y + headRectHalfHeight &&
				((smileRect.y + smileRect.height / 2 - headRect.y) / (float)headRect.height) > min_smile_center_y_percentage)
			{
				return true;
			}
		}
		return false;
	}

public:	
	SmileDetector(VideoCapture & video_capture, int bufferSize = 1) : video_capture(video_capture)
	{
		if (!SmileDetector::face_cascade.load("haarcascades/haarcascade_frontalface_default.xml"))
			throw exception("Could not load face cascade");

		if (!SmileDetector::smile_cascade.load("haarcascades/haarcascade_smile.xml"))
			throw exception("Could not load eye cascade");

		SmileDetector::video_capture.set(CV_CAP_PROP_BUFFERSIZE, bufferSize);
	}

	~SmileDetector()
	{
	}

	Rect GetSmile(Mat & gray_img)
	{
		SmileDetector::face_cascade.detectMultiScale(gray_img, SmileDetector::head_rects);

		if (SmileDetector::head_rects.size() == 0)
		{
			return Rect();
		}

		SmileDetector::smile_cascade.detectMultiScale(gray_img, SmileDetector::smile_rects);

		int maxSize = -1;
		int index = -1;
		int i = 0;

		while (i < SmileDetector::smile_rects.size())
		{
			if (SmileDetector::smile_is_valid(SmileDetector::smile_rects[i]))
			{
				int size = SmileDetector::smile_rects[i].area();
				if (size > maxSize)
				{
					maxSize = size;
					index = i;
				}
				++i;
			}
			else
			{
				SmileDetector::smile_rects.erase(SmileDetector::smile_rects.begin() + i);
			}
		}

		if (index == -1)
		{
			return Rect();
		}

		return SmileDetector::smile_rects[index];
	}
};

