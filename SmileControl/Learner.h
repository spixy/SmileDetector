#pragma once

#include "utility.h"
#include <iostream>

class Learner
{

private:
	VideoCapture & video_capture;
	SmileDetector * detector;

	bool try_capture_face(bool post_process, Mat & gray_img)
	{
		Rect smile_rect;
		Mat rgb_img;

		do
		{
			// capture image
			Learner::video_capture >> rgb_img;

			// process image
			flip(rgb_img, rgb_img, 1);
			cvtColor(rgb_img, gray_img, COLOR_BGR2GRAY);
			equalizeHist(gray_img, gray_img);

			// get smile Rect
			smile_rect = Learner::detector->GetSmile(gray_img);

			rectangle(rgb_img, smile_rect, smile_rect_color);

			imshow(window_rgb, rgb_img);

			if (waitKey(5) == KEY_ESC)
				return false;

		} while (smile_rect.area() == 0);

		gray_img = post_process ? process_gray_image(gray_img(smile_rect)) : gray_img(smile_rect);
		return true;
	}

public:

	Learner(VideoCapture & video_capture) : video_capture(video_capture)
	{
	}

	bool Learn(string smile_path, string sad_path, bool post_process)
	{
		try
		{
			Learner::detector = new SmileDetector(video_capture);
		}
		catch (exception ex)
		{
			cerr << ex.what() << endl;
			return false;
		}

		cout << "Press any key to capture your happy face." << endl;
		waitKey();

		Mat gray_img;
		bool saved = false;

		while (!saved)
		{
			cout << "Capturing your happy face, please wait..." << endl;

			if (!Learner::try_capture_face(post_process, gray_img))
				return false;

			cout << "Face captured, continue or retry? Press key C to continue." << endl;
			switch (waitKey())
			{
				case 'c':
				case 'C':
					imwrite(smile_path, gray_img);
					saved = true;
					break;
			}
		}

		saved = false;

		while (!saved)
		{
			cout << "Capturing your sad face, please wait..." << endl;

			if (!Learner::try_capture_face(post_process, gray_img))
				return false;

			cout << "Face captured, continue or retry? Press key C to continue." << endl;
			switch (waitKey())
			{
				case 'c':
				case 'C':
					imwrite(sad_path, gray_img);
					saved = true;
					break;
			}
		}

		delete detector;
		return true;
	}
};