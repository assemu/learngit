#ifndef _OBJECTINFO_H_
#define _OBJECTINFO_H_

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

struct Object
{
	cv::Rect_<float> rect;
	int label;
	float prob;
};

struct GridAndStride
{
	int grid0;
	int grid1;
	int stride;
};




#endif //_OBJECTINFO_H_
