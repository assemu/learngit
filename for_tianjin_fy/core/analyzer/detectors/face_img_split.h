#ifndef NANHANG_FACE_IMAGE_SPLIT_H_
#define NANHANG_FACE_IMAGE_SPLIT_H_

#include <opencv2/opencv.hpp>
#include <vector>

void Cut_face_img(cv::Mat src_img,
                  int win_size,
                  std::vector<cv::Mat>* ceil_img,
                  std::vector<double>* ceil_num,
                  double binary_threshold);

void Cut_face_img(cv::Mat src_img,
	int win_size,
	std::vector<cv::Mat>* ceil_img,
	std::vector<double>* ceil_num,
	double binary_threshold,
	int position);

#endif // NANHANG_FACE_IMAGE_SPLIT_H_
