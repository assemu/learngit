#ifndef BIOCONF_GUI_ACCOUNT_MANAGE_DIALOG_H_
#define BIOCONF_GUI_ACCOUNT_MANAGE_DIALOG_H_

#include <QDialog>

namespace Ui
{
class AccountManageDialog;
}

class AccountManageDialog : public QDialog
{
  Q_OBJECT;
public:
  AccountManageDialog();
  ~AccountManageDialog();

  void reload();

private:
  std::string getSelectedUser();

  Ui::AccountManageDialog* ui;
};

#endif //BIOCONF_GUI_ACCOUNT_MANAGE_DIALOG_H_
