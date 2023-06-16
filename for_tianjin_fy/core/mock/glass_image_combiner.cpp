#include "glass_image_combiner.h"

static cv::Mat _combine(const std::vector<PartImg>& parts, float scale_factor)
{
  cv::Mat re;
  int N = parts.size();
  if(N > 0) {
    //缩放
    std::vector<cv::Mat> imgs;
    for(auto& p : parts) {
      int resized_width = (int)(p.img.cols * scale_factor);
      int resized_height = (int)(p.img.rows * scale_factor);
      cv::Mat resized_img;
      resize(p.img, resized_img, cv::Size(resized_width, resized_height), cv::INTER_AREA);
      cv::cvtColor(resized_img, resized_img, cv::COLOR_GRAY2RGB); //原图是灰度图
      cv::bitwise_not(resized_img, resized_img); //反相
      imgs.push_back(resized_img);
    }

    auto pimg = imgs[0];
    int w = pimg.cols;
    int h = pimg.rows;
    re = cv::Mat::zeros(h * N, w, CV_8UC3);
    cv::cvtColor(re, re, cv::COLOR_BGR2RGB);
    for(int i = 0; i < N; i++) {
      auto& im = imgs[i];
      im.copyTo(re(cv::Rect(0, h * i, w, h)));
    }
  }
  return re;
}

GlassImageCombiner::GlassImageCombiner(float scale_factor)
  : _scale_factor(scale_factor)
{
}

static void _get_off(const cv::Mat& img, bool is_left, int& itop, int& margin)
{
  cv::Mat gray_img;
  cv::cvtColor(img, gray_img, cv::COLOR_RGB2GRAY);

  //对图像做开运算
  cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
  cv::Mat binary;
  cv::morphologyEx(gray_img, binary, cv::MORPH_OPEN, kernel); //开操作
  cv::Canny(binary, binary, 10, 100);

  //4.提取外轮廓
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(binary, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);

  int height = img.rows;
  int width = img.cols;
  float w_det_th = 0.1;
  float m_det_th = 0.5;

  itop = height;
  margin = 0;
  if(is_left)
    margin = width;

  for(int k = 0; k < (int)(width * m_det_th); k++) {
    int x_mar = is_left ? k : width - k;
    for(int i = 0; i < contours.size(); i++) {
      for(int j = 0; j < contours[i].size(); j++) {
        if(contours[i][j].x == x_mar) {
          margin = x_mar;
          break;
        }
      }

      if(margin == x_mar)
        break;
    }
  }

  for(int k = 0; k < (int)(width * w_det_th); k++) {
    int x_top = is_left ? width - k : k;
    for(int i = 0; i < contours.size(); i++) {
      for(int j = 0; j < contours[i].size(); j++) {
        if(contours[i][j].x == x_top) {
          if(contours[i][j].y < itop) {
            itop = contours[i][j].y;
          }
        }
      }
    }
    if(itop != height)
      break;
  }
}

cv::Mat GlassImageCombiner::getCombinedImg(int& left_margin, int& right_margin)
{
  int N = _left_part_imgs.size();
  int M = _right_part_imgs.size();
  if(N + M > 0) { //1.如果有新的PartImg，先填充到左右图像中
    if(N > 0) {
      QMutexLocker locker(&_mutex);
      auto cimg = _combine(_left_part_imgs, _scale_factor);
      cv::Mat nimg = cv::Mat::zeros(_left_img.rows + cimg.rows, cimg.cols, CV_8UC3);
      cv::cvtColor(nimg, nimg, cv::COLOR_BGR2RGB);
      if(!_left_img.empty())
        _left_img.copyTo(nimg(cv::Rect(0, 0, _left_img.cols, _left_img.rows)));
      cimg.copyTo(nimg(cv::Rect(0, _left_img.rows, cimg.cols, cimg.rows)));
      _left_img = nimg;
      _left_part_imgs.clear();
    }

    if(M > 0) {
      QMutexLocker locker(&_mutex);
      auto cimg = _combine(_right_part_imgs, _scale_factor);
      cv::Mat nimg = cv::Mat::zeros(_right_img.rows + cimg.rows, cimg.cols, CV_8UC3);
      cv::cvtColor(nimg, nimg, cv::COLOR_BGR2RGB);
      if(!_right_img.empty())
        _right_img.copyTo(nimg(cv::Rect(0, 0, _right_img.cols, _right_img.rows)));
      cimg.copyTo(nimg(cv::Rect(0, _right_img.rows, cimg.cols, cimg.rows)));
      _right_img = nimg;
      _right_part_imgs.clear();
    }

    //2.合并左右图像
    {
      auto limg = _left_img;
      auto rimg = _right_img;

      //切除重叠部分
      {
        //TODO
      }

      int left_height = limg.rows;
      int right_height = rimg.rows;
      int left_width = limg.cols;
      int right_width = rimg.cols;

      int H = std::max(limg.rows, rimg.rows);
      int W = limg.cols + rimg.cols;
      cv::Mat nimg = cv::Mat::zeros(H, W, CV_8UC3);
      cv::cvtColor(nimg, nimg, cv::COLOR_BGR2RGB);

      int left_top = 0;
      int w_left = 0;
      _get_off(limg, true, left_top, w_left);

      int right_top = 0;
      int w_right = 0;
      _get_off(rimg, true, right_top, w_right);

      //偏移处理
      //if(!(abs(right_top - left_top) > 400 || (left_top == left_height) || (right_top == right_height))) {
      if(!((left_top == left_height) || (right_top == right_height))) {
        cv::Mat M = (cv::Mat_<float>(2, 3) <<
                     1, 0, 0,
                     0, 1, right_top - left_top);
        cv::warpAffine(limg, limg, M, cv::Size(left_width, left_height), cv::INTER_LINEAR);//仿射变换
      }

      //拼合
      limg.copyTo(nimg(cv::Rect(0, 0, limg.cols, limg.rows)));
      rimg.copyTo(nimg(cv::Rect(limg.cols, 0, rimg.cols, rimg.rows)));
      cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
      cv::dilate(nimg, _img, kernel); //膨胀操作

      //输出
      _mar_left = w_left;
      _mar_right = right_width - w_right;
    }
  }

  //输出左右边界
  left_margin = _mar_left;
  right_margin = _mar_right;
  //返回合并的图像
  return _img;
}

void GlassImageCombiner::addLeft(const PartImg& img)
{
  QMutexLocker locker(&_mutex);
  _left_part_imgs.push_back(img);
}

void GlassImageCombiner::addRight(const PartImg& img)
{
  QMutexLocker locker(&_mutex);
  _right_part_imgs.push_back(img);
}
