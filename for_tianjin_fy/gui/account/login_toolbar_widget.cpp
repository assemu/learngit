#include "login_toolbar_widget.h"
#include "login_dialog.h"
#include <QtWidgets>
#include <core/account/account_manager.h>

LoginToolBarWidget::LoginToolBarWidget()
  : QWidget()
{
  auto icon_label = new QLabel();
  icon_label->setPixmap(QPixmap(":icons/user.png"));
  name_label = new QLabel(tr("操作员：默认用户"));
  btn = new QPushButton(QIcon(":icons/login.png"), tr("登录"));
  connect(btn, SIGNAL(clicked()), this, SIGNAL(login()));

  auto ly = new QHBoxLayout();
  ly->setContentsMargins(0, 5, 0, 5);
  ly->addWidget(icon_label);
  ly->addWidget(name_label);
  ly->addWidget(btn);
  setLayout(ly);
}

void LoginToolBarWidget::logined()
{
  auto account = AccountManager::get()->currentLoginedAccount();
  std::string username = "默认用户";
  if(account)
    username = account->name;
  name_label->setText(tr("操作员：") + QString::fromStdString(username));
  btn->setEnabled(false);
  name_label->update();
  btn->update();
}
