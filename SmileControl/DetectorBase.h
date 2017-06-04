#pragma once

#include <opencv2/opencv.hpp>

class DetectorBase
{
public:
	DetectorBase() {}

	virtual ~DetectorBase() {}

	virtual void Process(cv::Mat& img) = 0;
};

