#ifndef BIOCONF_GUI_LOGIN_DIALOG_H_
#define BIOCONF_GUI_LOGIN_DIALOG_H_

#include <QDialog>

namespace Ui
{
class LoginDialog;
}

class LoginDialog : public QDialog
{
  Q_OBJECT;
public:
  LoginDialog();
  ~LoginDialog();

private:
  Ui::LoginDialog* ui;
};

#endif //BIOCONF_GUI_LOGIN_DIALOG_H_
