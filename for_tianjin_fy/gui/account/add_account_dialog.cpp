#include "add_account_dialog.h"
#include "ui_add_account_dialog.h"
#include <core/account/account_manager.h>
#include <gui/util/message_box_helper.h>
#include <QtWidgets>

AddAccountDialog::AddAccountDialog()
  : QDialog(), ui(new Ui::AddAccountDialog)
{
  ui->setupUi(this);
  ui->passEdit->setEchoMode(QLineEdit::Password);
  ui->pass2Edit->setEchoMode(QLineEdit::Password);
  setTabOrder(ui->userNameEdit, ui->passEdit);
  setTabOrder(ui->passEdit, ui->pass2Edit);
  ui->userNameEdit->setFocus();

  connect(ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    QString userName = ui->userNameEdit->text().trimmed();
    QString pass = ui->passEdit->text().trimmed();
    QString pass2 = ui->pass2Edit->text().trimmed();

    bool found = false;
    for(const auto& a : AccountManager::get()->loadAllAccounts()) {
      if(a.name == userName.toStdString()) {
        found = true;
        break;
      }
    }

    if(found) {
      MessageBoxHelper::show_critical(tr("账号已经存在！"));
      return;
    }

    if(pass != pass2) {
      MessageBoxHelper::show_critical(tr("两次输入密码不一致！"));
      return;
    }

    if(pass.isEmpty()) {
      MessageBoxHelper::show_critical(tr("密码不能为空！"));
      return;
    }

    _userName = userName.toStdString();
    _pass = pass.toStdString();
    done(1);
  });

  connect(ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    done(0);
  });
}

AddAccountDialog::~AddAccountDialog()
{
  delete ui;
}
