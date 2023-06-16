#ifndef ALGO_LOCAL_RUN_DIALOG_H_
#define ALGO_LOCAL_RUN_DIALOG_H_

#include <memory>
#include <QDialog>
#include <QMutex>
#include <QQueue>
#include <QPointF>

class QPushButton;
class cvMatShowWidget;

namespace Ui
{
class AlgoLocalRunDialog;
}

class AlgoLocalRunDialog : public QDialog
{
public:
  AlgoLocalRunDialog();
  ~AlgoLocalRunDialog();

private:
  Ui::AlgoLocalRunDialog* _ui;
  cvMatShowWidget* _cw;
};

#endif //ALGO_LOCAL_RUN_DIALOG_H_
