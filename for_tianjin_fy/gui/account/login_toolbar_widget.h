#ifndef BIOCONF_GUI_LOGIN_TOOLBAR_WIDGET_H_
#define BIOCONF_GUI_LOGIN_TOOLBAR_WIDGET_H_

#include <QWidget>

class QLabel;
class QPushButton;

class LoginToolBarWidget : public QWidget
{
  Q_OBJECT;
public:
  LoginToolBarWidget();

  void logined();

signals:
  void login();

private:
  QLabel* name_label;
  QPushButton* btn;
};

#endif //BIOCONF_GUI_LOGIN_TOOLBAR_WIDGET_H_
