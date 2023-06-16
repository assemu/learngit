#ifndef BIOCONF_GUI_ACCOUNT_ADMIN_CHECK_DIALOG_H_
#define BIOCONF_GUI_ACCOUNT_ADMIN_CHECK_DIALOG_H_

#include <QDialog>

namespace Ui
{
class AdminCheckDialog;
}

class AdminCheckDialog : public QDialog
{
  Q_OBJECT;
public:
  AdminCheckDialog();
  ~AdminCheckDialog();

private:
  Ui::AdminCheckDialog* ui;
};

#endif //BIOCONF_GUI_ACCOUNT_ADMIN_CHECK_DIALOG_H_
