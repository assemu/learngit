#ifndef GUI_PART_NUM_CONFIG_WIDGET_H_
#define GUI_PART_NUM_CONFIG_WIDGET_H_

#include <QWidget>
#include <core/common.h>

namespace Ui
{
class PartNumConfigWidget;
}

class PartNumConfigWidget : public QWidget
{
  Q_OBJECT;
public:
  PartNumConfigWidget();
  ~PartNumConfigWidget();

  std::map<CameraPosition, int> getBeginSteps();
  void setBeginSteps(const std::map<CameraPosition, int>& c);

  std::map<CameraPosition, int> getEndSteps();
  void setEndSteps(const std::map<CameraPosition, int>& c);

  std::map<CameraPosition, int> getSteps();
  void setSteps(const std::map<CameraPosition, int>& c);

signals:
  void paramChanged();

private:
  Ui::PartNumConfigWidget* _ui;
};

#endif //GUI_PART_NUM_CONFIG_WIDGET_H_
