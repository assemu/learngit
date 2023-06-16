#include "edit_account_pass_dialog.h"
#include "ui_edit_account_pass_dialog.h"
#include <core/account/account_manager.h>
#include <gui/util/message_box_helper.h>
#include <QtWidgets>

EditAccountPassDialog::EditAccountPassDialog(const std::string userName)
  : QDialog(), ui(new Ui::EditAccountPassDialog), _userName(userName)
{
  ui->setupUi(this);
  ui->userNameLabel->setText(QString::fromStdString(userName));
  ui->passEdit->setEchoMode(QLineEdit::Password);
  ui->pass2Edit->setEchoMode(QLineEdit::Password);
  setTabOrder(ui->passEdit, ui->pass2Edit);
  ui->passEdit->setFocus();

  connect(ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    done(0);
  });

  connect(ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    QString pass = ui->passEdit->text().trimmed();
    QString pass2 = ui->pass2Edit->text().trimmed();

    if(pass != pass2) {
      MessageBoxHelper::show_critical(tr("两次输入密码不一致！"));
      return;
    }

    if(pass.isEmpty()) {
      MessageBoxHelper::show_critical(tr("密码不能为空！"));
      return;
    }

    done(1);
  });
}

std::string EditAccountPassDialog::getPass() const
{
  return ui->passEdit->text().trimmed().toStdString();
}

EditAccountPassDialog::~EditAccountPassDialog()
{
  delete ui;
}
