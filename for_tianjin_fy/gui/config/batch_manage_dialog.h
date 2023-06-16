#ifndef GUI_BATCH_MANAGE_DIALOG_H_
#define GUI_BATCH_MANAGE_DIALOG_H_

#include <QDialog>

namespace Ui
{
class BatchManageDialog;
}

class BatchManageDialog : public QDialog
{
public:
  BatchManageDialog();

  void reload();

private:
  Ui::BatchManageDialog* _ui;
};

#endif //GUI_BATCH_MANAGE_DIALOG_H_
