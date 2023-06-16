#ifndef V2_PART_IMG_SOURCE_H_
#define V2_PART_IMG_SOURCE_H_

#include <memory>
#include <core/common.h>

class PartImgConsumer;

//线扫图像采集器
class PartImgSource
{
public:
  virtual ~PartImgSource()
  {}

  //开始采集
  virtual bool startWork(GlassConfig glass_config) = 0;

  //停止采集
  virtual bool stopWork() = 0;

  //注册图像处理
  virtual void registerPartImgConsumer(PartImgConsumer* consumer) = 0;

  //注册图像处理对象
  virtual void unregisterPartImgConsumer(PartImgConsumer* consumer) = 0;
};

#endif //V2_PART_IMG_SOURCE_H_
