#ifndef GUI_MOTOR_DEBUG_DIALOG_H_
#define GUI_MOTOR_DEBUG_DIALOG_H_

#include <QDialog>

namespace Ui
{
class MotorDebugDialog;
}

class MotorDebugDialog : public QDialog
{
public:
  MotorDebugDialog();
  ~MotorDebugDialog();

  //过滤事件
  //@see QWidget
  bool event(QEvent* event) override;

private:
  void load();
  void _spin(int ms);

  Ui::MotorDebugDialog* _ui;
  double _pos3;
  double _pos4;
  double _pos_belt;
};

#endif //GUI_MOTOR_DEBUG_DIALOG_H_
