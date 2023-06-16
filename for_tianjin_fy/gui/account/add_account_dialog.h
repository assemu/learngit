#ifndef BIOCONF_GUI_ACCOUNT_ADD_ACCOUNT_DIALOG_H_
#define BIOCONF_GUI_ACCOUNT_ADD_ACCOUNT_DIALOG_H_

#include <QDialog>
#include <core/account/account.h>

namespace Ui
{
class AddAccountDialog;
}

class AddAccountDialog : public QDialog
{
  Q_OBJECT;
public:
  AddAccountDialog();
  ~AddAccountDialog();

  inline std::string getUserName() const
  {
    return _userName;
  }

  inline std::string getPass() const
  {
    return _pass;
  }

private:
  Ui::AddAccountDialog* ui;
  std::string _userName;
  std::string _pass;
};

#endif //BIOCONF_GUI_ACCOUNT_ADD_ACCOUNT_DIALOG_H_
