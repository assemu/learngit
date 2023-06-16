#ifndef GUI_THRESHOLDS_EDIT_WIDGET_H_
#define GUI_THRESHOLDS_EDIT_WIDGET_H_

#include <map>
#include <QTabWidget>
#include <core/common.h>

class EdgeCAMThresholdsWidget;
class FaceCAMThresholdsWidget;

class ThresholdsEditWidget : public QTabWidget
{
  Q_OBJECT;
public:
  ThresholdsEditWidget();

  void loadParam(const std::map<CameraPosition, CameraConfig>& c);
  void getParam(std::map<CameraPosition, CameraConfig>& c);

signals:
  void paramChanged();

private:
  std::map<CameraPosition, EdgeCAMThresholdsWidget*> ews;
  std::map<CameraPosition, FaceCAMThresholdsWidget*> fws;
};

#endif //GUI_THRESHOLDS_EDIT_WIDGET_H_
