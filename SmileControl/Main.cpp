#include "SmileDetector.h"
#include "SmileComparer.h"

using namespace cv;
using namespace std;

int start_app(VideoCapture & video_capture, string smilePath, string sadPath, bool post_process);
void process_result(SmileComparer::SmileResult result, SmileComparer::SmileResult last_result);
void learn(VideoCapture & video_capture, string smile_path, string sad_path, bool postProcess);
Mat process_gray_image(const Mat & img);

const int KEY_ESC = 27;
const string VER = "agah";
const string window_rgb = "window_rgb";
const string window_gray = "window_gray";
const string happy_template_path = "template_happy.jpg";
const string sad_template_path = "template_sad.jpg";
const Scalar smile_rect_color = Scalar(0, 255, 0);
const bool post_process_default = true;
int match_method;

int main(int argc, const char ** argv)
{
	cout << "Loading, v" << VER << endl;

	VideoCapture video_capture;
	video_capture.open(0);
	if (!video_capture.isOpened())
	{
		cerr << "VideoCapture not opened" << endl;
		return 1;
	}

	string smilePath = happy_template_path;
	string sadPath = sad_template_path;
	bool postProcess = post_process_default;

	namedWindow(window_rgb);
	namedWindow(window_gray);

	switch (argc)
	{
		case 2:
			if (strcmp(argv[1], "learn") == 0)
			{
				learn(video_capture, smilePath, sadPath, postProcess);
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
				learn(video_capture, smilePath, sadPath, postProcess);
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

int start_app(VideoCapture & video_capture, string smilePath, string sadPath, bool post_process)
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
		comparer = new SmileComparer(smilePath, sadPath);
	}
	catch (exception ex)
	{
		cerr << ex.what() << endl;
		learn(video_capture, smilePath, sadPath, post_process);
		delete detector;
		return 2;
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
			//Mat smile_image = post_process ? process_gray_image(gray_img(smile_rect)) : gray_img(smile_rect);

			//result = comparer->Compare(smile_image);

			result = comparer->Compare(gray_img);

			process_result(result, last_result);
		}
		else
		{
			result = SmileComparer::SmileResult::None;
		}

		imshow(window_rgb, rgb_img);
		imshow(window_gray, gray_img);

		last_result = result;

	} while (waitKey(10) != KEY_ESC);

	delete detector;
	delete comparer;

	return 0;
}

Mat process_gray_image(const Mat & img)
{
	Mat bw;
	adaptiveThreshold(~img, bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 15, -2);

	// Create the images that will use to extract the horizontal and vertical lines
	Mat horizontal = bw.clone();

	// Specify size on horizontal axis
	int horizontalsize = horizontal.cols / 30;

	// Create structure element for extracting horizontal lines through morphology operations
	Mat horizontalStructure = getStructuringElement(MORPH_RECT, Size(horizontalsize, 1));

	// Apply morphology operations
	erode(horizontal, horizontal, horizontalStructure);
	dilate(horizontal, horizontal, horizontalStructure);

	return horizontal;
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
		break;
	}
}

void learn(VideoCapture & video_capture, string smile_path, string sad_path, bool post_process)
{
	SmileDetector * detector;
	try
	{
		detector = new SmileDetector(video_capture);
	}
	catch (exception ex)
	{
		cerr << ex.what() << endl;
		exit(1);
	}

	Mat img;

	cout << "Press any key to capture your happy face." << endl;

	waitKey();

	cout << "Capturing your happy face, please wait..." << endl;

	while (1)
	{
		// capture image
		video_capture >> img;

		// process image
		flip(img, img, 1);
		cvtColor(img, img, COLOR_BGR2GRAY);
		equalizeHist(img, img);

		// get smile Rect
		Rect smile_rect = detector->GetSmile(img);

		rectangle(img, smile_rect, smile_rect_color);

		imshow(window_rgb, img);

		if (smile_rect.area() > 0)
		{
			Mat smile_image = post_process ? process_gray_image(img(smile_rect)) : img(smile_rect);
			imwrite(smile_path, smile_image);
			cout << "Saved." << endl;
			break;
		}

		waitKey(5);
	}

	cout << "Press any key to capture your sad face." << endl;

	waitKey();

	cout << "Capturing your sad face, please wait..." << endl;

	while (1)
	{
		// capture image
		video_capture >> img;

		// process image
		flip(img, img, 1);
		cvtColor(img, img, COLOR_BGR2GRAY);
		equalizeHist(img, img);

		// get smile Rect
		Rect smile_rect = detector->GetSmile(img);

		rectangle(img, smile_rect, smile_rect_color);

		imshow(window_rgb, img);

		if (smile_rect.area() > 0)
		{
			Mat smile_image = post_process ? process_gray_image(img(smile_rect)) : img(smile_rect);
			imwrite(sad_path, img);
			cout << "Saved." << endl;
			break;
		}

		waitKey(5);
	}

	delete detector;
}