#ifndef TENSORRT_YOLOX_DETECTOR2_H_
#define TENSORRT_YOLOX_DETECTOR2_H_

#include <memory>
#include <string>
#include <opencv2/opencv.hpp>
#include "tensorrt_yolo_detector.h"

class CSoftbagXInfer;

class TensorRTYoloXDetector2 : public TensorRTYoloDetector
{
public:
  /**
   * @brief 构造函数
   *
   * @param overlap_threshold 识别框重叠过滤阈值 一般设置为0.3
   * 识别后从前往后进行重叠过滤，后面识别框与之前识别框重叠达到该百分比将被过滤
   *
   * @param obj_threshold 检测概率阈值 一般设置为0.3
   */
  TensorRTYoloXDetector2(float overlap_threshold, float obj_threshold);

  ~TensorRTYoloXDetector2();

  //加载模型接口
  virtual bool load(const std::string& labels_file_path, const std::string& engine_file_path) override;

  //检测
  virtual bool detect(const cv::Mat& img, std::vector<BoxInfo>& boxes) override;

private:
  std::vector<std::string> class_names;
  std::shared_ptr<CSoftbagXInfer> _p;
  float _overlap_threshold;
  float _obj_threshold;
};

#endif //TENSORRT_YOLOX_DETECTOR2_H_
