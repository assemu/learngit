#ifndef GUI_EDGE_CAM_THREHOLDS_WIDGET_H_
#define GUI_EDGE_CAM_THREHOLDS_WIDGET_H_

#include <QWidget>

namespace Ui
{
class EdgeCAMThresholdsWidget;
}

class EdgeCAMThresholdsWidget : public QWidget
{
  Q_OBJECT;
public:
  EdgeCAMThresholdsWidget();
  ~EdgeCAMThresholdsWidget();

  void setConfidenceProbability(const std::string& label, float p);
  void setAreaThreshold(const std::string& label, float p);
  void setLengthThreshold(const std::string& label, float p);

  float getConfidenceProbability(const std::string& label);
  float getAreaThreshold(const std::string& label);
  float getLengthThreshold(const std::string& label);

  void setLabelFilterOut(const std::string& label, bool p);
  bool getLabelFilterOut(const std::string& label);

signals:
  void paramChanged();

private:
  Ui::EdgeCAMThresholdsWidget* _ui;
};

#endif //GUI_EDGE_CAM_THREHOLDS_WIDGET_H_
