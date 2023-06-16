#ifndef TENSORRT_YOLO_DETECTOR_H_
#define TENSORRT_YOLO_DETECTOR_H_

#include <memory>
#include <string>
#include <opencv2/opencv.hpp>
#include <core/common.h>


class TensorRTYoloDetector
{
public:
  //虚析构
  virtual ~TensorRTYoloDetector()
  {}

  //加载模型接口
  virtual bool load(const std::string& labels_file_path, const std::string& engine_file_path) = 0;

  /**
   * @brief 检测图片得到识别框
   *
   * @param img 输入图片
   * Yolo要求模型输入是640*640，识别前会首先进行缩放（保持比例，如果不是正方形空白处添加灰色）
   *
   * @param boxes 输出识别框列表
   *
   * @return 识别是否成功,如果出现异常返回false
   */
  virtual bool detect(const cv::Mat& img, std::vector<BoxInfo>& boxes) = 0;
};

#endif //TENSORRT_YOLO_DETECTOR_H_
