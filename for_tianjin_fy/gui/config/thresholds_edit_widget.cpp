#include "thresholds_edit_widget.h"
#include "edge_cam_thresholds_widget.h"
#include "face_cam_thresholds_widget.h"
#include <core/common.h>

ThresholdsEditWidget::ThresholdsEditWidget()
  : QTabWidget()
{
  for(auto position : allCameraPositions()) {
    if(isEdgePosition(position)) {
      auto tw = new EdgeCAMThresholdsWidget();
      ews[position] = tw;
      addTab(tw, QString("%1[边]").arg(QString::fromStdString(cameraNameOf(position))));
      connect(tw, SIGNAL(paramChanged()), this, SIGNAL(paramChanged()));
    } else {
      auto tw = new FaceCAMThresholdsWidget();
      fws[position] = tw;
      addTab(tw, QString("%1[面]").arg(QString::fromStdString(cameraNameOf(position))));
      connect(tw, SIGNAL(paramChanged()), this, SIGNAL(paramChanged()));
    }
  }
}

void ThresholdsEditWidget::loadParam(const std::map<CameraPosition, CameraConfig>& c)
{
  for(auto& kv : c) {
    auto position = kv.first;
    auto camera_config = kv.second;
    if(ews.count(position) > 0) {
      auto w = ews[position];
      for(auto& lv : camera_config.confidence_probabilities)
        w->setConfidenceProbability(lv.first, lv.second);
      for(auto& lv : camera_config.area_thresholds)
        w->setAreaThreshold(lv.first, lv.second);
      for(auto& lv : camera_config.length_thresholds)
        w->setLengthThreshold(lv.first, lv.second);
      for(auto& lv : camera_config.label_filter_outs)
        w->setLabelFilterOut(lv.first, lv.second);
    }
    if(fws.count(position) > 0) {
      auto w = fws[position];
      w->setFaceImgSplitBinaryThreshhold(camera_config.face_binary_split_thresholds);
      for(auto& lv : camera_config.confidence_probabilities)
        w->setConfidenceProbability(lv.first, lv.second);
      for(auto& lv : camera_config.area_thresholds)
        w->setAreaThreshold(lv.first, lv.second);
      for(auto& lv : camera_config.length_thresholds)
        w->setLengthThreshold(lv.first, lv.second);
      for(auto& lv : camera_config.label_filter_outs)
        w->setLabelFilterOut(lv.first, lv.second);
    }
  }
}

void ThresholdsEditWidget::getParam(std::map<CameraPosition, CameraConfig>& c)
{
  for(auto& kv : ews) {
    auto position = kv.first;
    auto w = kv.second;
    std::vector<std::string> labels = {"white", "black"};
    for(auto& label : labels) {
      c[position].confidence_probabilities[label] = w->getConfidenceProbability(label);
      c[position].area_thresholds[label] = w->getAreaThreshold(label);
      c[position].length_thresholds[label] = w->getLengthThreshold(label);
      c[position].label_filter_outs[label] = w->getLabelFilterOut(label);
    }
  }

  for(auto& kv : fws) {
    auto position = kv.first;
    auto w = kv.second;
    c[position].face_binary_split_thresholds = w->getFaceImgSplitBinaryThreshhold();
    std::vector<std::string> labels = { "bright", "dark", "dirty", "scratch", "circle" };
    for(auto& label : labels) {
      c[position].confidence_probabilities[label] = w->getConfidenceProbability(label);
      c[position].area_thresholds[label] = w->getAreaThreshold(label);
      c[position].length_thresholds[label] = w->getLengthThreshold(label);
      c[position].label_filter_outs[label] = w->getLabelFilterOut(label);
    }
  }
}
