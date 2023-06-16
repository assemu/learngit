#include "account_manage_dialog.h"
#include "ui_account_manage_dialog.h"
#include "add_account_dialog.h"
#include "edit_account_pass_dialog.h"
#include "edit_admin_pass_dialog.h"
#include <QtWidgets>
#include <core/account/account_manager.h>
#include <gui/util/message_box_helper.h>
#include <QDateTime>

AccountManageDialog::AccountManageDialog()
  : QDialog(), ui(new Ui::AccountManageDialog)
{
  ui->setupUi(this);

  auto table = ui->tableWidget;

  table->setStyleSheet("QTableView::item:selected:!active{background:#287399;color:#eff0f1;}");
  table->setColumnCount(4);

  //设置表头内容
  QStringList header;
  header << tr("ID") << tr("用户名") << tr("密码") << tr("上次登录时间");
  table->setHorizontalHeaderLabels(header);

  QHeaderView* hv = table->horizontalHeader();
  hv->setStretchLastSection(true);
  hv->setSectionResizeMode(QHeaderView::Interactive);
  hv->setDefaultSectionSize(100);
  hv->setDefaultAlignment(Qt::AlignLeft);
  hv->setFixedHeight(25);
  hv->setSectionsClickable(true);

  hv->resizeSection(0, 40);
  hv->resizeSection(1, 200);
  hv->resizeSection(2, 120);
  hv->resizeSection(3, 150);
  hv->setSectionResizeMode(0, QHeaderView::Fixed);
  hv->setSectionResizeMode(1, QHeaderView::Stretch);
  hv->setSectionResizeMode(2, QHeaderView::Fixed);
  hv->setSectionResizeMode(3, QHeaderView::Fixed);
  //hv->setStretchLastSection(false);

  table->verticalHeader()->setVisible(false);
  table->setShowGrid(false);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setAlternatingRowColors(true);

  ui->delBtn->setEnabled(false);
  ui->editPassBtn->setEnabled(false);

  reload();

  connect(table, &QTableWidget::itemSelectionChanged, this,
  [this]() {
    auto table = ui->tableWidget;
    auto list = table->selectedItems();
    if(list.size() > 0) {
      ui->delBtn->setEnabled(true);
      ui->editPassBtn->setEnabled(true);
    } else {
      ui->delBtn->setEnabled(false);
      ui->editPassBtn->setEnabled(false);
    }
  });

  connect(ui->addBtn, &QPushButton::clicked, this,
  [this]() {
    AddAccountDialog d;
    if(d.exec()) {
      auto userName = d.getUserName();
      auto pass = d.getPass();
      if(AccountManager::get()->addAccount(userName, pass)) {
        reload();
        MessageBoxHelper::show_info(tr("添加成功!"));
      }
    }
  });

  connect(ui->delBtn, &QPushButton::clicked, this,
  [this]() {
    auto name = getSelectedUser();
    if(!name.empty()) {
      if(MessageBoxHelper::show_confirm(tr("确认删除用户:") + QString::fromStdString(name))) {
        if(AccountManager::get()->deleteAccount(name)) {
          reload();
          MessageBoxHelper::show_info(tr("删除成功！"));
        }
      }
    }
  });

  connect(ui->editPassBtn, &QPushButton::clicked, this,
  [this]() {
    auto name = getSelectedUser();
    if(!name.empty()) {
      EditAccountPassDialog d(name);
      if(d.exec()) {
        auto pass = d.getPass();
        if(AccountManager::get()->changeAccountPass(name, pass)) {
          reload();
          MessageBoxHelper::show_info(tr("修改密码成功！"));
        }
      }
    }
  });

  connect(ui->editAdminPassBtn, &QPushButton::clicked, this,
  [this]() {
    EditAdminPassDialog d;
    if(d.exec()) {
      auto old_pass = d.getOldPass();
      auto new_pass = d.getNewPass();
      if(AccountManager::get()->changeAdminPass(old_pass, new_pass)) {
        reload();
        MessageBoxHelper::show_info(tr("修改管理员密码成功！"));
      }
    }
  });
}

AccountManageDialog::~AccountManageDialog()
{
  delete ui;
}

void AccountManageDialog::reload()
{
  auto table = ui->tableWidget;
  table->setRowCount(0);

  auto list = AccountManager::get()->loadAllAccounts();
  int i = 0;
  for(const auto& a : list) {
    table->setRowCount(table->rowCount() + 1);
    int r = table->rowCount() - 1;
    table->setRowHeight(r, 25);

    //ID
    i++;
    {
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText(QString("%1").arg(i));
      table->setItem(r, 0, it);
    }

    //用户名
    {
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText(QString::fromStdString(a.name));
      table->setItem(r, 1, it);
    }

    //密码
    {
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText("********");
      table->setItem(r, 2, it);
    }

    //上次登录时间
    {
      auto d = QDateTime::fromMSecsSinceEpoch(a.last_login_timestamp);
      auto str = d.toString("yyyy.MM.dd hh:mm:ss");
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText(str);
      table->setItem(r, 3, it);
    }
  }
}

std::string AccountManageDialog::getSelectedUser()
{
  std::string re;
  auto table = ui->tableWidget;
  auto list = table->selectedItems();
  for(const auto& it : list) {
    if(it != nullptr && it->column() == 1) {
      re = it->text().trimmed().toStdString();
    }
  }
  return re;
}
