#pragma once

#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const string window_rgb = "window_rgb";
const string happy_template_path = "template_happy.jpg";
const string sad_template_path = "template_sad.jpg";
const Scalar smile_rect_color = Scalar(0, 255, 0);
const bool post_process_default = true;
const bool show_camera_default = true;
const bool process_once_default = true;
const int KEY_ESC = 27;

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