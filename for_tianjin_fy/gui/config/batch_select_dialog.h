#ifndef GUI_BATCH_SELECT_DIALOG_H_
#define GUI_BATCH_SELECT_DIALOG_H_

#include <QDialog>

namespace Ui
{
class BatchSelectDialog;
}

class BatchSelectDialog : public QDialog
{
public:
  BatchSelectDialog();

  void reload();

  QString getBatchId();         //批号ID

private:
  Ui::BatchSelectDialog* _ui;
};

#endif //GUI_BATCH_SELECT_DIALOG_H_
