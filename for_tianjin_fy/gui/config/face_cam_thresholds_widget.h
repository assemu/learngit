#ifndef GUI_FACE_CAM_THREHOLDS_WIDGET_H_
#define GUI_FACE_CAM_THREHOLDS_WIDGET_H_

#include <QWidget>

namespace Ui
{
class FaceCAMThresholdsWidget;
}

class FaceCAMThresholdsWidget : public QWidget
{
  Q_OBJECT;
public:
  FaceCAMThresholdsWidget();
  ~FaceCAMThresholdsWidget();

  void setConfidenceProbability(const std::string& label, float p);
  void setAreaThreshold(const std::string& label, float p);
  void setLengthThreshold(const std::string& label, float p);

  float getConfidenceProbability(const std::string& label);
  float getAreaThreshold(const std::string& label);
  float getLengthThreshold(const std::string& label);

  void setFaceImgSplitBinaryThreshhold(float v);
  float getFaceImgSplitBinaryThreshhold();

  void setLabelFilterOut(const std::string& label, bool p);
  bool getLabelFilterOut(const std::string& label);

signals:
  void paramChanged();

private:
  Ui::FaceCAMThresholdsWidget* _ui;
};

#endif //GUI_FACE_CAM_THREHOLDS_WIDGET_H_
