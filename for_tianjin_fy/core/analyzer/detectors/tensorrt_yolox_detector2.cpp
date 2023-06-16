#include "tensorrt_yolox_detector2.h"
#include <NvOnnxParser.h>
#include "private/buffers.h"
#include "private/common.h"
#include "private/buffers.h"
#include <core/util/str_util.h>
#include "test/CSoftbagXInfer.h"

//------------------------------------------------------

//------------------------------------------------------

TensorRTYoloXDetector2::TensorRTYoloXDetector2(float overlap_threshold, float obj_threshold)
  : _overlap_threshold(overlap_threshold), _obj_threshold(obj_threshold)
{
  _p = std::make_shared<CSoftbagXInfer>();
}

TensorRTYoloXDetector2::~TensorRTYoloXDetector2()
{
}

bool TensorRTYoloXDetector2::detect(const cv::Mat& img, std::vector<BoxInfo>& boxes)
{
  std::vector<Object> objs;
  _p->detect(img, objs);

  boxes.clear();
  for(auto& b : objs) {
    boxes.push_back(BoxInfo{ cv::Rect(b.rect.x, b.rect.y, b.rect.width, b.rect.height),
                             b.prob,
                             class_names[b.label] });
  }
  return true;
}

bool TensorRTYoloXDetector2::load(const std::string& labels_file_path, const std::string& engine_file_path)
{
  std::ifstream ifs(labels_file_path);
  std::string line;
  while(getline(ifs, line)) {
    str_trim(line);
    if(!line.empty())
      this->class_names.push_back(line);
  }

  _p->load(_overlap_threshold, _obj_threshold, engine_file_path.c_str());

  return true;
}
