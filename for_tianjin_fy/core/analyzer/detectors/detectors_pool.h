#ifndef V2_ANALYZER_DETECTORS_POOL_H_
#define V2_ANALYZER_DETECTORS_POOL_H_

#include <string>
#include <core/util/thread_safe_queue.h>
#include "tensorrt_yolo_detector.h"

class DetectorsPool final
{
public:
  static DetectorsPool* get();
  ~DetectorsPool();

  //检测侧扫图片
  bool detectEdgeImg(const cv::Mat& img, std::vector<BoxInfo>& boxes);

  //检测面扫图片
  bool detectFaceImg(const cv::Mat& img, std::vector<BoxInfo>& boxes);

private:
  DetectorsPool();
  ThreadSafeQueue<std::shared_ptr<TensorRTYoloDetector>> _edge_detectors;
  ThreadSafeQueue<std::shared_ptr<TensorRTYoloDetector>> _face_detectors;
};

#endif //V2_ANALYZER_DETECTORS_POOL_H_
