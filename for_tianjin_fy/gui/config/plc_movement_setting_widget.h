#ifndef GUI_PLC_MOVEMENT_SETTING_WIDGET_H_
#define GUI_PLC_MOVEMENT_SETTING_WIDGET_H_

#include <QWidget>
#include <core/plc/plc_config.h>

namespace Ui
{
class PLCMovementWidget;
}

class LineChartWidget;

class PLCMovementSettingWidget : public QWidget
{
  Q_OBJECT;
public:
  PLCMovementSettingWidget();
  ~PLCMovementSettingWidget();

  void setConfig(const PlcConfigTrack& c);
  void getConfig(PlcConfigTrack& c);

  bool event(QEvent* event) override;

private:
  Ui::PLCMovementWidget* _ui;
  LineChartWidget* _chart3;
  LineChartWidget* _chart4;
  std::vector<double> _cam3_data;
  std::vector<double> _cam4_data;
};

#endif // GUI_PLC_MOVEMENT_SETTING_WIDGET_H_
