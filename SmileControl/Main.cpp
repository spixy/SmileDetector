#include "EyeDetector.h"
#include "SmileDetector.h"

using namespace cv;
using namespace std;

const string WINDOW = "window";
const int KEY_ESC = 27;
const int VER = 1;

int main(int argc, const char** argv)
{
	DetectorBase* detector;

	try
	{
		detector = new SmileDetector(true);
	}
	catch (std::exception ex)
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

	do
	{
		cap >> image;

		detector->Process(image);

		flip(image, image, 1);
		imshow(WINDOW, image);

	} while (waitKey(10) != KEY_ESC);

	delete detector;

	return 0;
}