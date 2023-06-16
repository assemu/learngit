#ifndef BIOCONF_GUI_ACCOUNT_EDIT_ACCOUNT_PASS_DIALOG_H_
#define BIOCONF_GUI_ACCOUNT_EDIT_ACCOUNT_PASS_DIALOG_H_

#include <QDialog>

namespace Ui
{
class EditAccountPassDialog;
}

class EditAccountPassDialog : public QDialog
{
  Q_OBJECT;
public:
  EditAccountPassDialog(const std::string userName);
  ~EditAccountPassDialog();

  std::string getPass() const;

private:
  Ui::EditAccountPassDialog* ui;
  std::string _userName;
};

#endif //BIOCONF_GUI_ACCOUNT_EDIT_ACCOUNT_PASS_DIALOG_H_
