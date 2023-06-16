#include "detectors_pool.h"
#include <core/app_config.h>
#include <core/util/log.h>
#include <core/util/filesystem_util.h>
#include "tensorrt_util.h"
#include "tensorrt_yolov5_detector.h"
#include "tensorrt_yolox_detector.h"
#include "tensorrt_yolox_detector2.h"

DetectorsPool* DetectorsPool::get()
{
  static DetectorsPool* _inst = NULL;
  if(_inst == NULL)
    _inst = new DetectorsPool();
  return _inst;
}

DetectorsPool::~DetectorsPool()
{
}

DetectorsPool::DetectorsPool()
{
  auto model_dir = AppConfig::get()->getAppHomeDir() + "/model";

  std::string edge_onnx_file_path = bin_dir() + "/edge_detect_model.onnx";
  std::string edge_engine_file_path = model_dir + "/edge_detect_tensorrt_engine.bin";

  std::string face_onnx_file_path = bin_dir() + "/face_detect_model.onnx";
  std::string face_engine_file_path = model_dir + "/face_detect_tensorrt_engine.bin";

  TensorRTUtil::convertOnnxToTensorRT(edge_onnx_file_path, edge_engine_file_path);
  TensorRTUtil::convertOnnxToTensorRT(face_onnx_file_path, face_engine_file_path);

  //创建N个边缘检测器
  for(int i = 0; i < 6; i++) {
    //auto d = std::make_shared<TensorRTYoloV5Detector>(0.3, 0.45);
    auto d = std::make_shared<TensorRTYoloXDetector>(0.3, 0.3);
    if(d->load(bin_dir() + "/edge_detect_labels.txt", edge_engine_file_path)) {
      _edge_detectors.push_back(d);
    }
  }

  //创建N个面检测器
  for(int i = 0; i < 6; i++) {
    //auto d = std::make_shared<TensorRTYoloXDetector>(0.3, 0.45);
    auto d = std::make_shared<TensorRTYoloXDetector>(0.3, 0.3);
    if(d->load(bin_dir() + "/face_detect_labels.txt", face_engine_file_path)) {
      _face_detectors.push_back(d);
    }
  }
}

bool DetectorsPool::detectEdgeImg(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  if(img.empty()) {
    log_warn("detect empty edge image!");
    return false;
  }

  auto d = _edge_detectors.wait_and_pop_front();
  bool re = d->detect(img, boxes);
  _edge_detectors.push_back(d);
  return re;
}

bool DetectorsPool::detectFaceImg(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  if(img.empty()) {
    log_warn("detect empty face image!");
    return false;
  }

  auto d = _face_detectors.wait_and_pop_front();
  bool re = d->detect(img, boxes);
  _face_detectors.push_back(d);
  return re;
}
