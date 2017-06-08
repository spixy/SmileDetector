#pragma once

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class SmileComparer
{

private:
	Mat happy_template, sad_template;
	TemplateMatchModes match_method;

	bool cmp(const Mat & img, const Mat & templ, Mat & result, Point & matchLoc, Point & size)
	{
		int result_cols = img.cols - templ.cols + 1;
		int result_rows = img.rows - templ.rows + 1;

		if (result_cols <= 0 || result_rows <= 0)
		{
			matchLoc = Point();
			size = Point();
			return false;
		}

		result.create(result_rows, result_cols, CV_32FC1);
		matchTemplate(img, templ, result, match_method);

		normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

		double minVal; double maxVal; Point minLoc; Point maxLoc;

		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

		if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED)
		{
			matchLoc = minLoc;
		}
		else
		{
			matchLoc = maxLoc;
		}

		size = Point(matchLoc.x + templ.cols, matchLoc.y + templ.rows);
		return true;
	}

public:
	enum SmileResult
	{
		None,
		Happy,
		Sadness
	};

	SmileComparer(string smile_path, string sad_path, TemplateMatchModes template_mode = TemplateMatchModes::TM_SQDIFF) : match_method(template_mode)
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


	SmileResult Compare(const Mat & gray_img)
	{
		Point matchLoc1, matchLoc2, size1, size2;
		Mat result;

		SmileComparer::cmp(gray_img, SmileComparer::happy_template, result, matchLoc1, size1);
		SmileComparer::cmp(gray_img, SmileComparer::sad_template, result, matchLoc2, size2);

		if (size1.x * size1.y > size2.x * size2.y)
		{
			return SmileResult::Happy;
		}
		else if (size1.x * size1.y < size2.x * size2.y)
		{
			return SmileResult::Sadness;
		}
		else
		{
			return SmileResult::None;
		}
	}
};