#ifndef JIANFENG_CORE_GLASS_IMAGE_COMBINER_H_
#define JIANFENG_CORE_GLASS_IMAGE_COMBINER_H_

#include <vector>
#include <core/common.h>
#include <opencv2/opencv.hpp>
#include <QMutex>


class GlassImageCombiner
{
public:
  GlassImageCombiner(float scale_factor);

  cv::Mat getCombinedImg(int& left_margin, int& right_margin);

  void addLeft(const PartImg& img);
  void addRight(const PartImg& img);

private:
  float _scale_factor;
  std::vector<PartImg> _left_part_imgs;
  std::vector<PartImg> _right_part_imgs;
  cv::Mat _left_img;
  cv::Mat _right_img;
  cv::Mat _img;
  int _mar_left;
  int _mar_right;
  QMutex _mutex;
};

#endif //JIANFENG_CORE_GLASS_IMAGE_COMBINER_H_
