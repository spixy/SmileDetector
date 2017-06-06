#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class SmileComparer
{

private:
	Mat happy_template, sad_template, result;

	void find_and_draw_contours(Mat & gray_img)
	{
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;

		// Detect edges using canny
		Canny(gray_img, gray_img, 100, 200, 3);

		const Scalar edge_color = Scalar(255, 0, 0);
		
		// Find contours
		findContours(gray_img, contours, hierarchy, RetrievalModes::RETR_LIST, ContourApproximationModes::CHAIN_APPROX_SIMPLE);

		size_t length = contours.size();

		// Find max contour
		size_t max = 0;
		int index = -1;
		for (int i = 0; i < length; i++)
		{
			size_t size = contours[i].size();
			if (size > max)
			{
				max = size;
				index = i;
			}
		}

		// Draw max contour
		drawContours(gray_img, contours, index, edge_color, 1, 8, hierarchy, 0);
	}

	void fastMatchTemplate(Mat& srca,		// The reference image
						   Mat& srcb,		// The template image
						   Mat& dst,		// Template matching result
						   int maxlevel)	// Number of levels
	{
		vector<Mat> refs, tpls, results;

		// Build Gaussian pyramid
		buildPyramid(srca, refs, maxlevel);
		buildPyramid(srcb, tpls, maxlevel);

		Mat ref, tpl, res;

		// Process each level
		for (int level = maxlevel; level >= 0; level--)
		{
			ref = refs[level];
			tpl = tpls[level];

			Size size = ref.size() + Size(1, 1) - tpl.size();

			/*if (size.width <= 0 || size.height <= 0)
				continue;*/

			res = Mat::zeros(size, CV_32FC1);

			if (level == maxlevel)
			{
				// On the smallest level, just perform regular template matching
				matchTemplate(ref, tpl, res, CV_TM_CCORR_NORMED);
			}
			else
			{
				// On the next layers, template matching is performed on pre-defined 
				// ROI areas.  We define the ROI using the template matching result 
				// from the previous layer.

				Mat mask;
				pyrUp(results.back(), mask);

				Mat mask8u;
				mask.convertTo(mask8u, CV_8U);

				// Find matches from previous layer
				vector<vector<Point> > contours;
				findContours(mask8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

				// Use the contours to define region of interest and 
				// perform template matching on the areas
				for (int i = 0; i < contours.size(); i++)
				{
					Rect r = boundingRect(contours[i]);
					matchTemplate(
						ref(r + (tpl.size() - cv::Size(1, 1))),
						tpl,
						res(r),
						CV_TM_CCORR_NORMED
					);
				}
			}

			// Only keep good matches
			threshold(res, res, 0.80, 1., CV_THRESH_TOZERO);
			results.push_back(res);
		}

		res.copyTo(dst);
	}

	bool match(Mat & ref, Mat & tpl)
	{
		Mat res_32f(ref.rows - tpl.rows + 1, ref.cols - tpl.cols + 1, CV_32FC1);
		matchTemplate(ref, tpl, res_32f, CV_TM_CCOEFF_NORMED);

		Mat res;
		res_32f.convertTo(res, CV_8U, 255.0);
		imshow("result", res);

		int size = ((tpl.cols + tpl.rows) / 4) * 2 + 1; //force size to be odd
		adaptiveThreshold(res, res, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, size, -128);
		imshow("result_thresh", res);

		while (true)
		{
			double minval, maxval, threshold = 0.8;
			Point minloc, maxloc;
			minMaxLoc(res, &minval, &maxval, &minloc, &maxloc);

			if (maxval >= threshold)
			{
				rectangle(ref, maxloc, Point(maxloc.x + tpl.cols, maxloc.y + tpl.rows), CV_RGB(0, 255, 0), 2);
				floodFill(res, maxloc, 0); //mark drawn blob
				return true;
			}
			else
				break;
		}

		return false;
	}

	Mat happy_dst;
	Mat sad_dst;

public:
	enum SmileResult
	{
		None,
		Happy,
		Sadness
	};

	SmileComparer(string smile_path, string sad_path)
	{
		SmileComparer::happy_template = imread(smile_path, IMREAD_GRAYSCALE);
		if (SmileComparer::happy_template.data == nullptr)
			throw exception("Coult not load happy template image");

		SmileComparer::sad_template = imread(sad_path, IMREAD_GRAYSCALE);
		if (SmileComparer::sad_template.data == nullptr)
			throw exception("Coult not load sad template image");

		equalizeHist(SmileComparer::happy_template, SmileComparer::happy_template);
		equalizeHist(SmileComparer::sad_template, SmileComparer::sad_template);
	}

	~SmileComparer()
	{
	}


	SmileResult Compare(Mat & gray_img)
	{
		bool v1 = match(gray_img, SmileComparer::happy_template);
		bool v2 = match(gray_img, SmileComparer::sad_template);

		cout << v1 << " , " << v2;

		return SmileResult::None;

		//fastMatchTemplate(SmileComparer::happy_template, gray_img, happy_dst, 2);
		//fastMatchTemplate(SmileComparer::sad_template, gray_img, sad_dst, 2);

		/*double minval, maxval1, maxval2;
		Point minloc, maxloc1, maxloc2, maxloc;
		Mat tpl;
		SmileResult result;

		minMaxLoc(happy_dst, &minval, &maxval1, &minloc, &maxloc1);

		minMaxLoc(sad_dst, &minval, &maxval2, &minloc, &maxloc2);

		if (maxval1 > maxval2)
		{
			if (maxval1 < 0.5)
				return SmileResult::None;

			maxloc = maxloc1;
			tpl = SmileComparer::happy_template;
			result = SmileResult::Happy;
		}
		else
		{
			if (maxval2 < 0.5)
				return SmileResult::None;

			maxloc = maxloc2;
			tpl = SmileComparer::sad_template;
			result = SmileResult::Sadness;
		}

		cout << maxval1 << " , " << maxval2;

		rectangle(gray_img, maxloc, Point(maxloc.x + tpl.cols, maxloc.y + tpl.rows), CV_RGB(0, 255, 0), 2);
		floodFill(gray_img, maxloc, Scalar(0), 0, Scalar(.1), Scalar(1.));

		return result;*/

		/*vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		find_and_draw_contours(gray_img, contours, hierarchy);
		imshow("0", gray_img);

		double d1 = SmileComparer::compare(gray_img, SmileComparer::happy_template);
		imshow("1", gray_img);

		double d2 = SmileComparer::compare(gray_img, SmileComparer::sad_template);
		imshow("2", gray_img);*/


		//cout << "d1: " << d1 << "d2: " << d2;

		/*if (d1 <= 0 && d2 <= 0)
		{
			return SmileResult::None;
		}
		else if (d1 > d2)
		{
			return SmileResult::Happy;
		}
		else //if (d2 > d1)
		{
			return SmileResult::Sadness;
		}*/
	}
	
};