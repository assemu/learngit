#ifndef GUI_BATCH_CREATE_DIALOG_H_
#define GUI_BATCH_CREATE_DIALOG_H_

#include <QDialog>

namespace Ui
{
class BatchCreateDialog;
}

class BatchCreateDialog : public QDialog
{
public:
  BatchCreateDialog();

  QString getBatchId();         //批号ID
  QString getGlassConfigName(); //型号名称

private:
  Ui::BatchCreateDialog* _ui;
};

#endif //GUI_BATCH_CREATE_DIALOG_H_
