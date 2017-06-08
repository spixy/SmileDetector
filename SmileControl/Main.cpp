#include "SmileDetector.h"
#include "SmileComparer.h"
#include "utility.h"
#include "Learner.h"

using namespace cv;
using namespace std;

int start_app(VideoCapture & video_capture, string smilePath, string sadPath, bool post_process, bool process_once, bool show_camera);
SmileComparer::SmileResult get_result(SmileComparer::SmileResult result, SmileComparer::SmileResult last_result);

const int CODE_HAPPY = 200;
const int CODE_SAD = 400;

int main(int argc, const char ** argv)
{
	VideoCapture videoCapture;
	videoCapture.open(0);

	if (!videoCapture.isOpened())
	{
		cerr << "VideoCapture not opened" << endl;
		return 4;
	}

	string smilePath = happy_template_path;
	string sadPath = sad_template_path;
	bool postProcess = post_process_default;
	bool processOnce = process_once_default;
	bool showCamera = show_camera_default;
	
	switch (argc)
	{
		case 2:
			if (strcmp(argv[1], "learn") == 0)
			{
				Learner learner(videoCapture);
				if (!learner.Learn(smilePath, sadPath, postProcess))
					return 1;
			}
			else if (strcmp(argv[1], "once") == 0)
			{
				processOnce = true;
			}
			else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "/?") == 0 || strcmp(argv[1], "--help") == 0)
			{
				cout << "USAGE:" << endl;
				cout << "  help  - displays help" << endl;
				cout << "  learn - captures happy and sad templates" << endl;
				cout << "  once  - stops after captured face state, returns 0 - happy, 1 - sad or 2 - error" << endl;
				cout << "  nocam - to not show capture window, used with parameter 'once' only" << endl;
				cout << "  HAPPY_PATH SAD_PATH - overides template paths, used with parameter 'learn' or none" << endl;
			}
			break;

		case 3:
			if (strcmp(argv[1], "once") == 0 && strcmp(argv[2], "nocam") == 0)
			{
				processOnce = true;
				showCamera = false;
			}
			else
			{
				smilePath = string(argv[1]);
				sadPath = string(argv[2]);
			}
			break;

		case 4:
			if (strcmp(argv[1], "learn") == 0)
			{
				smilePath = string(argv[2]);
				sadPath = string(argv[3]);

				Learner learner(videoCapture);
				if (!learner.Learn(smilePath, sadPath, postProcess))
					return 1;
			}
			break;
	}

	int result;
	do
	{
		result = start_app(videoCapture, smilePath, sadPath, postProcess, processOnce, showCamera);
	} while (result == 2);

	if (processOnce)
	{
		switch (result)
		{
			case CODE_HAPPY:
				return 0;

			case CODE_SAD:
				return 1;

			default: // error
				return 2;
		}
	}

	return result;
}

int start_app(VideoCapture & video_capture, string smile_path, string sad_path, bool post_process, bool process_once, bool show_camera)
{
	SmileDetector * detector;
	try
	{
		detector = new SmileDetector(video_capture);
	}
	catch (exception ex)
	{
		cerr << ex.what() << endl;
		return 1;
	}

	SmileComparer * comparer;
	try
	{
		comparer = new SmileComparer(smile_path, sad_path);
	}
	catch (exception ex)
	{
		delete detector;
		cerr << ex.what() << endl;

		Learner learner(video_capture);
		if (learner.Learn(smile_path, sad_path, post_process))
			return 2; // app is restarting
		else
			return 1;
	}
	
	Mat rgb_img, gray_img;
	SmileComparer::SmileResult result = SmileComparer::SmileResult::None;
	SmileComparer::SmileResult last_result = SmileComparer::SmileResult::None;

	do
	{
		// capture image
		video_capture >> rgb_img;

		// process image
		flip(rgb_img, rgb_img, 1);
		cvtColor(rgb_img, gray_img, COLOR_BGR2GRAY);
		equalizeHist(gray_img, gray_img);

		// get smile Rect
		Rect smile_rect = detector->GetSmile(rgb_img);

		rectangle(rgb_img, smile_rect, smile_rect_color);

		if (smile_rect.area() > 0)
		{
			Mat smile_image = post_process ? process_gray_image(gray_img(smile_rect)) : gray_img(smile_rect);
			result = comparer->Compare(smile_image);
		}
		else
		{
			result = SmileComparer::SmileResult::None;
		}

		switch (get_result(result, last_result))
		{
			case SmileComparer::SmileResult::Happy:
				if (process_once)
				{
					delete detector;
					delete comparer;
					return CODE_HAPPY;
				}
				else
				{
					cout << ":)" << endl;
				}
				break;

			case SmileComparer::SmileResult::Sadness:
				if (process_once)
				{
					delete detector;
					delete comparer;
					return CODE_SAD;
				}
				else
				{
					cout << ":(" << endl;
				}
				break;

			case SmileComparer::SmileResult::None:
				if (!process_once)
				{
					cout << ":(" << endl;
				}
				break;
		}

		if (show_camera)
		{
			imshow(window_rgb, rgb_img);
		}

		last_result = result;

	} while (waitKey(10) != KEY_ESC);

	delete detector;
	delete comparer;

	return 0;
}

SmileComparer::SmileResult get_result(SmileComparer::SmileResult result, SmileComparer::SmileResult last_result)
{
	if (result == SmileComparer::None)
	{
		return last_result;
	}
	else
	{
		return result;
	}
}
