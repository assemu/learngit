#ifndef V2_PART_IMG_CONSUMER_H_
#define V2_PART_IMG_CONSUMER_H_

#include <core/common.h>

//线扫图像处理
class PartImgConsumer
{
public:
  virtual ~PartImgConsumer()
  {}

  //接收侧面线扫图像,可在不同线程中调用
  //需要立即返回，不能影响调用线程
  virtual void acceptEdgePartImg(const PartImg& img) = 0;

  //接收正反面线扫图像
  virtual void acceptFacePartImg(const PartImg& img) = 0;
};

#endif //V2_PART_IMG_CONSUMER_H_
