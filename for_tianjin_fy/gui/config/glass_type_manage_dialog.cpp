#include "glass_type_manage_dialog.h"
#include "ui_glass_type_manage_dialog.h"
#include "glass_type_config_edit_dialog.h"
#include <QtWidgets>
#include <core/db/db.h>
#include <core/util/log.h>
#include <gui/util/message_box_helper.h>

GlassTypeManageDialog::GlassTypeManageDialog()
  : QDialog(), _ui(new Ui::GlassTypeManageDialog)
{
  setWindowTitle("玻璃型号管理");

  _ui->setupUi(this);

  auto table = _ui->tableWidget;
  table->setStyleSheet("QTableView::item:selected:!active{background:#287399;color:#eff0f1;}");
  table->setColumnCount(1);
  //设置表头内容
  QStringList header;
  header << tr("型号名称");
  table->setHorizontalHeaderLabels(header);

  QHeaderView* hv = table->horizontalHeader();
  hv->setStretchLastSection(true);
  hv->setSectionResizeMode(QHeaderView::Interactive);
  hv->setDefaultSectionSize(200);
  hv->setDefaultAlignment(Qt::AlignLeft);
  hv->setFixedHeight(25);
  hv->setSectionsClickable(true);
  hv->setSectionResizeMode(0, QHeaderView::Stretch);
  //hv->setStretchLastSection(false);

  table->verticalHeader()->setVisible(false);
  table->setShowGrid(false);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setAlternatingRowColors(true);

  _ui->editBtn->setEnabled(false);

  reload();

  connect(table, &QTableWidget::itemSelectionChanged, this,
  [this]() {
    auto table = _ui->tableWidget;
    auto list = table->selectedItems();
    if(list.size() > 0) {
      _ui->editBtn->setEnabled(true);
    } else {
      _ui->editBtn->setEnabled(false);
    }
  });

  connect(_ui->addBtn, &QPushButton::clicked, this,
  [this]() {
    GlassConfig re;
    GlassTypeConfigEditDialog d;
    d.loadParam(re);
    if(d.exec()) {
      d.getParam(re);
      if(!DB::get()->isGlassConfigExist(re.name)) {
        log_debug("saving GlassConfig:{} to db ...", re.name);
        if(DB::get()->saveGlassConfig(re)) {
          log_debug("GlassConfig:{} saved!", re.name);
          reload();
          MessageBoxHelper::show_info("添加型号配置成功！");
        }
      }
    }
  });

  connect(_ui->editBtn, &QPushButton::clicked, this,
  [this]() {
    auto name = getSelectedConfig();
    auto dao = DB::get();
    if(!name.empty()) {
      GlassConfig re;
      if(dao->queryGlassConfigByName(name, re)) {
        GlassTypeConfigEditDialog d;
        d.disableEditName();
        d.loadParam(re);
        if(d.exec()) {
          d.getParam(re);
          if(dao->saveGlassConfig(re)) {
            reload();
            MessageBoxHelper::show_info("修改型号配置成功！");
          }
        }
      }
    }
  });
}

void GlassTypeManageDialog::reload()
{
  auto table = _ui->tableWidget;
  table->setRowCount(0);

  auto list = DB::get()->queryAllGlassConfigNames();
  int i = 0;
  for(const auto& a : list) {
    table->setRowCount(table->rowCount() + 1);
    int r = table->rowCount() - 1;
    table->setRowHeight(r, 25);

    //型号名称
    i++;
    {
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText(QString::fromStdString(a));
      table->setItem(r, 0, it);
    }
  }
}

std::string GlassTypeManageDialog::getSelectedConfig()
{
  std::string re;
  auto table = _ui->tableWidget;
  auto list = table->selectedItems();
  for(const auto& it : list) {
    if(it != nullptr && it->column() == 0) {
      re = it->text().trimmed().toStdString();
    }
  }
  return re;
}
