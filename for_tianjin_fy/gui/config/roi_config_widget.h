#ifndef GUI_ROI_CONFIG_WIDGET_H_
#define GUI_ROI_CONFIG_WIDGET_H_

#include <QWidget>
#include <core/common.h>

namespace Ui
{
class RoiConfigWidget;
}

class RoiConfigWidget : public QWidget
{
  Q_OBJECT;
public:
  RoiConfigWidget();
  ~RoiConfigWidget();

  void setRoiConfig(const std::map<CameraPosition, cv::Rect>& c);
  std::map<CameraPosition, cv::Rect> getRoiConfig();

signals:
  void paramChanged();

private:
  Ui::RoiConfigWidget* _ui;
};

#endif //GUI_ROI_CONFIG_WIDGET_H_
