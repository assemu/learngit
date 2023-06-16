#include "login_dialog.h"
#include "ui_login_dialog.h"
#include <QtWidgets>
#include <core/account/account_manager.h>
#include <gui/util/message_box_helper.h>

LoginDialog::LoginDialog()
  : QDialog(), ui(new Ui::LoginDialog)
{
  ui->setupUi(this);
  ui->passEdit->setEchoMode(QLineEdit::Password);

  connect(ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    auto userName = ui->userNameEdit->text().trimmed().toStdString();
    auto pass = ui->passEdit->text().trimmed().toStdString();
    if(AccountManager::get()->login(userName, pass)) {
      done(1);
    } else {
      MessageBoxHelper::show_critical(tr("账号或者密码错误！\n请重新登录"));
    }
  });

  connect(ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    done(0);
  });
}

LoginDialog::~LoginDialog()
{
  delete ui;
}
