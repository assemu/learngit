#include "face_img_split.h"
#include <opencv2/opencv.hpp>
#include <vector>

void Cut_face_img(cv::Mat src_img,
                  int win_size,
                  std::vector<cv::Mat>* ceil_img,
                  std::vector<double>* ceil_num,
                  double binary_threshold)
{
  int height = src_img.rows;
  int width = src_img.cols;

  int ceil_height = height;
  int ceil_width = win_size;
  cv::Mat roi_img;
  int win_num = width / win_size;
  std::vector<double> nums;
  std::vector<cv::Mat> tmp_ceil_img;
  std::vector<double> tmp_ceil_num;

  for(int i = 0; i < win_num; i++) {
    roi_img = src_img(cv::Rect(i * ceil_width, 0, ceil_width, ceil_height));
    //imshow("src_img", roi_img);
    //waitKey(0);
    cv::Mat meanResult, stddevResult;
    double std;
    meanStdDev(roi_img, meanResult, stddevResult);
    std = stddevResult.at<double>(0, 0);
    //cout << std << endl;
    if(std > 1.7) {
      nums.push_back(i);
    }
  }
  /*
  for (int i = 0; i < nums.size(); i++)
  {
  	cout << nums[i] << " ";
  }
  cout << endl;
  */
  for(int i = 0; i < nums.size(); i++) {
    double k = nums[i];
    cv::Mat dst_img;
    //最后一张图片
    if(i == nums.size() - 1) {
      src_img(cv::Rect(k * ceil_width, 0, ceil_width, ceil_height)).copyTo(dst_img);
      //imshow("src_img", dst_img);
      //waitKey(0);
      tmp_ceil_img.push_back(dst_img);
      tmp_ceil_num.push_back(k);
      break;
    }
    //300
    if(nums[i + 1] != k + 1) {
      src_img(cv::Rect(k * ceil_width, 0, ceil_width, ceil_height)).copyTo(dst_img);
      //imshow("src_img", dst_img);
      //waitKey(0);
      tmp_ceil_img.push_back(dst_img);
      tmp_ceil_num.push_back(k);
    }
    //600
    else {
      double num = k;
      cv::Mat tmp_img;
      src_img(cv::Rect(k * ceil_width, 0, ceil_width * 2, ceil_height)).copyTo(tmp_img);
      //imshow("src_img", tmp_img);
      //waitKey(0);
      int w_begin = 0;
      int w_end = ceil_width * 2;
      int cut_size = 30;
      for(; w_end - w_begin > win_size; w_begin += cut_size) {
        roi_img = tmp_img(cv::Rect(w_begin, 0, cut_size, ceil_height));
        cv::Mat meanResult, stddevResult;
        double std;
        meanStdDev(roi_img, meanResult, stddevResult);
        std = stddevResult.at<double>(0, 0);
        if(std > 1.7) {
          break;
        } else {
          k += 0.1;
        }
      }
      for(; w_end - w_begin > win_size; w_end -= cut_size) {
        roi_img = tmp_img(cv::Rect(w_end - cut_size, 0, cut_size, ceil_height));
        cv::Mat meanResult, stddevResult;
        double std;
        meanStdDev(roi_img, meanResult, stddevResult);
        std = stddevResult.at<double>(0, 0);
        if(std > 1.7) {
          break;
        }
      }
      tmp_img(cv::Rect(w_begin, 0, w_end - w_begin, ceil_height)).copyTo(dst_img);
      //imshow("src_img", dst_img);
      //waitKey(0);
      tmp_ceil_img.push_back(dst_img);
      tmp_ceil_num.push_back(k);
      i++;
    }
  }

  for(int i = 0; i < tmp_ceil_img.size(); i++) {
    cv::Mat binary;
    cv::Mat canny;
    std::vector<cv::Vec2f> lines;
    //imshow("src", tmp_ceil_img[i]);
    threshold(tmp_ceil_img[i], binary, binary_threshold, 255, cv::THRESH_BINARY);
    //imshow("binary", binary);
    Canny(binary, canny, 0, 138);
    //imshow("canny", canny);
    HoughLines(canny, lines, 1.0, CV_PI / 180, 50);
    //cout << "hhhhh" << lines.size() << endl;
    if(lines.size() != 0) {
      ceil_img->push_back(tmp_ceil_img[i]);
      ceil_num->push_back(tmp_ceil_num[i]);
    }
    //waitKey(0);
  }
}

void Cut_face_img(cv::Mat src_img,
                  int win_size,
                  std::vector<cv::Mat>* ceil_img,
                  std::vector<double>* ceil_num,
                  double binary_threshold,
                  int position)
{
  if(!src_img.data) return;
  cv::Mat src = src_img.clone();
  //cv::bitwise_not(src, src);
  int width = src.cols;
  int ws = win_size;
  double num = ceil((double)width / ws);

  cv::Mat ROI(ws, ws, src.type());

  for(int i = 0; i < (int)num; i++) {
    //std::cout << i << " 3..." << src.cols << " " << src.rows << std::endl;

    auto rect1 = cv::Rect(i * ws, 0, ws, ws);
    auto rect2 = cv::Rect(0, 0, src.cols, src.rows);
    auto rect = rect1 & rect2;
    ROI = src(rect);

    // push back
    ceil_img->push_back(ROI);
    ceil_num->push_back(i);
  }
}
