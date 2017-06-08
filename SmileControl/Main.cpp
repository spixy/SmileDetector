#include "SmileDetector.h"
#include "SmileComparer.h"
#include "utility.h"
#include "Learner.h"

using namespace cv;
using namespace std;

int start_app(VideoCapture & video_capture, string smilePath, string sadPath, bool post_process);
void process_result(SmileComparer::SmileResult result, SmileComparer::SmileResult last_result);

int main(int argc, const char ** argv)
{
	VideoCapture video_capture;
	video_capture.open(0);

	if (!video_capture.isOpened())
	{
		cerr << "VideoCapture not opened" << endl;
		return 4;
	}

	string smilePath = happy_template_path;
	string sadPath = sad_template_path;
	bool postProcess = post_process_default;

	namedWindow(window_rgb);

	switch (argc)
	{
		case 2:
			if (strcmp(argv[1], "learn") == 0)
			{
				Learner learner(video_capture);
				if (!learner.Learn(smilePath, sadPath, postProcess))
					return 1;
			}
			break;

		case 3:
			smilePath = string(argv[1]);
			sadPath = string(argv[2]);
			break;

		case 4:
			if (strcmp(argv[1], "learn") == 0)
			{
				smilePath = string(argv[2]);
				sadPath = string(argv[3]);

				Learner learner(video_capture);
				if (!learner.Learn(smilePath, sadPath, postProcess))
					return 1;
			}
			break;
	}

	int result;
	do
	{
		result = start_app(video_capture, smilePath, sadPath, postProcess);
	} while (result == 2);

	return result;
}

int start_app(VideoCapture & video_capture, string smile_path, string sad_path, bool post_process)
{
	SmileDetector * detector;
	try
	{
		detector = new SmileDetector(video_capture);
	}
	catch (exception ex)
	{
		cerr << ex.what() << endl;
		return 3;
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
			return 2;
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

			process_result(result, last_result);
		}
		else
		{
			result = SmileComparer::SmileResult::None;
		}

		imshow(window_rgb, rgb_img);

		last_result = result;

	} while (waitKey(10) != KEY_ESC);

	delete detector;
	delete comparer;

	return 0;
}

void process_result(SmileComparer::SmileResult result, SmileComparer::SmileResult last_result)
{
	switch (result)
	{
		case SmileComparer::None:
			if (last_result == SmileComparer::Happy)
			{
				cout << ":)" << endl;
			}
			else if (last_result == SmileComparer::Sadness)
			{
				cout << ":(" << endl;
			}
			else
			{
				cout << ":|" << endl;
			}
			break;

		case SmileComparer::Happy:
			cout << ":)" << endl;
			break;

		case SmileComparer::Sadness:
			cout << ":(" << endl;
			break;

		default:
			cerr << "Error: Incorrect data" << endl;
			break;
	}
}
