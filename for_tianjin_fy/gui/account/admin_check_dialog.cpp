#include "admin_check_dialog.h"
#include "ui_admin_check_dialog.h"
#include <core/account/account_manager.h>
#include <gui/util/message_box_helper.h>
#include <QtWidgets>

AdminCheckDialog::AdminCheckDialog()
  : QDialog(), ui(new Ui::AdminCheckDialog)
{
  ui->setupUi(this);

  ui->passEdit->setEchoMode(QLineEdit::Password);

  connect(ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    auto pass = ui->passEdit->text().trimmed().toStdString();
    if(AccountManager::get()->checkAdminPass(pass)) {
      done(1);
    } else {
      MessageBoxHelper::show_critical(tr("管理员密码错误！\n请重新输入"));
    }
  });

  connect(ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    done(0);
  });
}

AdminCheckDialog::~AdminCheckDialog()
{
  delete ui;
}
