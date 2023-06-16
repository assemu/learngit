#ifndef GUI_PLC_DEVICE_RUN_DIALOG_H_
#define GUI_PLC_DEVICE_RUN_DIALOG_H_

#include <QDialog>
#include <QLabel>
#include <map>

namespace Ui
{
class PLCDeviceRunDialog;
}

class CircleLabel;

class PLCDeviceRunDialog : public QDialog
{
public:
  PLCDeviceRunDialog();
  ~PLCDeviceRunDialog();

  //过滤事件
  //@see QWidget
  bool event(QEvent* event) override;

private:
  void load();

  Ui::PLCDeviceRunDialog* _ui;
  std::map<std::string, bool> _err_map;
  std::map<std::string, int> _err_code_map;
  std::map<std::string, CircleLabel*> _err_labels;
  std::map<std::string, QLabel*> _err_code_labels;
};

#endif //GUI_PLC_DEVICE_RUN_DIALOG_H_
