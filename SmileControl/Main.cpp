#include "EyeDetector.h"
#include "SmileDetector.h"

using namespace cv;
using namespace std;

const string WINDOW = "window";
const int KEY_ESC = 27;
const int VER = 2;

int main(int argc, const char ** argv)
{
	DetectorBase * detector;

	try
	{
		detector = new SmileDetector();
	}
	catch (exception ex)
	{
		cerr << ex.what() << endl;
		return 1;
	}

	cout << "Loading, v" << VER << endl;

	Mat image;
	VideoCapture cap;
	cap.open(0);

	if (!cap.isOpened())
	{
		cerr << "VideoCapture not opened" << endl;
		return 0;
	}

	namedWindow(WINDOW, 1);

	clock_t timer = 0;
	unsigned long frames = 0;
	double last_time = 0, sum_time = 0;

	do
	{
		cap >> image;

		timer = std::clock();

		detector->Process(image);

		last_time = (std::clock() - timer) / (double)(CLOCKS_PER_SEC / 1000);
		sum_time += last_time;

		flip(image, image, 1);
		imshow(WINDOW, image);

		cout << "Frame times: last: " << last_time << ", average: " << (sum_time / ++frames) << " ms" << endl;

	} while (waitKey(10) != KEY_ESC);

	delete detector;

	return 0;
}