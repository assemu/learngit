#ifndef BIOCONF_GUI_ACCOUNT_EDIT_ADMIN_PASS_DIALOG_H_
#define BIOCONF_GUI_ACCOUNT_EDIT_ADMIN_PASS_DIALOG_H_

#include <QDialog>

namespace Ui
{
class EditAdminPassDialog;
}

class EditAdminPassDialog : public QDialog
{
  Q_OBJECT;
public:
  EditAdminPassDialog();
  ~EditAdminPassDialog();

  std::string getOldPass() const;
  std::string getNewPass() const;

private:
  Ui::EditAdminPassDialog* ui;
};

#endif //BIOCONF_GUI_ACCOUNT_EDIT_ADMIN_PASS_DIALOG_H_
