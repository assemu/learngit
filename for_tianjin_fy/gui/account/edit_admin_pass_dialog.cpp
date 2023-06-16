#include "edit_admin_pass_dialog.h"
#include "ui_edit_admin_pass_dialog.h"
#include <core/account/account_manager.h>
#include <gui/util/message_box_helper.h>
#include <QtWidgets>

EditAdminPassDialog::EditAdminPassDialog()
  : QDialog(), ui(new Ui::EditAdminPassDialog)
{
  ui->setupUi(this);
  ui->oldPassEdit->setEchoMode(QLineEdit::Password);
  ui->newPassEdit->setEchoMode(QLineEdit::Password);
  ui->newPass2Edit->setEchoMode(QLineEdit::Password);
  setTabOrder(ui->oldPassEdit, ui->newPassEdit);
  setTabOrder(ui->newPassEdit, ui->newPass2Edit);
  ui->oldPassEdit->setFocus();

  connect(ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    done(0);
  });

  connect(ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    QString old_pass = ui->oldPassEdit->text().trimmed();
    QString new_pass = ui->newPassEdit->text().trimmed();
    QString new_pass2 = ui->newPass2Edit->text().trimmed();

    auto AM = AccountManager::get();

    if(!AM->checkAdminPass(old_pass.toStdString())) {
      MessageBoxHelper::show_critical(tr("管理员密码不正确！"));
      return;
    }

    if(new_pass != new_pass2) {
      MessageBoxHelper::show_critical(tr("两次输入密码不一致！"));
      return;
    }

    if(new_pass.isEmpty()) {
      MessageBoxHelper::show_critical(tr("密码不能为空！"));
      return;
    }

    done(1);
  });
}

EditAdminPassDialog::~EditAdminPassDialog()
{
  delete ui;
}

std::string EditAdminPassDialog::getOldPass() const
{
  return ui->oldPassEdit->text().trimmed().toStdString();
}

std::string EditAdminPassDialog::getNewPass() const
{
  return ui->newPassEdit->text().trimmed().toStdString();
}
