#ifndef GUI_PLC_PEIFANG_SETTING_WIDGET_H_
#define GUI_PLC_PEIFANG_SETTING_WIDGET_H_

#include <QWidget>
#include <map>
#include <core/plc/plc_config.h>

namespace Ui
{
class PLCPeiFangWidget;
}

class QSpinBox;
class QLineEdit;

class PLCPeiFangSettingWidget : public QWidget
{
  Q_OBJECT;
public:
  PLCPeiFangSettingWidget();

  void setConfig(const PlcConfigRecipe& c);
  void getConfig(PlcConfigRecipe& c);

private:
  Ui::PLCPeiFangWidget* _ui;

  std::map<std::string, QSpinBox*> _spin_boxes1;
  std::map<std::string, QSpinBox*> _spin_boxes2;
  std::map<std::string, QLineEdit*> _line_edits;
};

#endif // GUI_PLC_PEIFANG_SETTING_WIDGET_H_
