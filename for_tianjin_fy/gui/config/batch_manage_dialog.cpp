#include "batch_manage_dialog.h"
#include "batch_create_dialog.h"
#include "ui_batch_manage_dialog.h"
#include <core/db/db.h>
#include <gui/util/message_box_helper.h>

BatchManageDialog::BatchManageDialog()
  : QDialog(), _ui(new Ui::BatchManageDialog)
{
  setWindowTitle("批号管理");
  _ui->setupUi(this);

  auto table = _ui->tableWidget;
  table->setStyleSheet("QTableView::item:selected:!active{background:#287399;color:#eff0f1;}");
  table->setColumnCount(2);
  //设置表头内容
  QStringList header;
  header << tr("批号") << tr("型号名称");
  table->setHorizontalHeaderLabels(header);

  QHeaderView* hv = table->horizontalHeader();
  hv->setStretchLastSection(true);
  hv->setSectionResizeMode(QHeaderView::Interactive);
  hv->setDefaultSectionSize(200);
  hv->setDefaultAlignment(Qt::AlignLeft);
  hv->setFixedHeight(25);
  hv->setSectionsClickable(true);

  hv->resizeSection(0, 200);
  hv->setSectionResizeMode(0, QHeaderView::Fixed);
  hv->setSectionResizeMode(1, QHeaderView::Stretch);
  //hv->setStretchLastSection(false);

  table->verticalHeader()->setVisible(false);
  table->setShowGrid(false);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setAlternatingRowColors(true);

  reload();

  connect(_ui->addBtn, &QPushButton::clicked, this,
  [this]() {
    BatchCreateDialog d;
    if(d.exec()) {
      auto batch_id = d.getBatchId().toStdString();
      auto glass_config_name = d.getGlassConfigName().toStdString();

      if(!DB::get()->isBatchExist(batch_id)) {
        GlassConfig gc;
        if(DB::get()->queryGlassConfigByName(glass_config_name, gc)) {
          Batch b;
          b.id = batch_id;
          b.glass_config = gc;
          if(DB::get()->addBatch(b)) {
            reload();
            MessageBoxHelper::show_info("批号ID创建成功！");
            return;
          } else {
            MessageBoxHelper::show_info("创建批号失败！");
            return;
          }
        } else {
          MessageBoxHelper::show_info("玻璃型号不存在！");
          return;
        }
      } else {
        MessageBoxHelper::show_info("批号已存在！");
        return;
      }
    }
  });
}

void BatchManageDialog::reload()
{
  auto table = _ui->tableWidget;
  table->setRowCount(0);

  auto list = DB::get()->queryAllBatches();
  int i = 0;
  for(const auto& b : list) {
    table->setRowCount(table->rowCount() + 1);
    int r = table->rowCount() - 1;
    table->setRowHeight(r, 25);

    //型号名称
    i++;
    {
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText(QString::fromStdString(b.id));
      table->setItem(r, 0, it);
    }

    //型号名称
    {
      auto it = new QTableWidgetItem();
      it->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      it->setText(QString::fromStdString(b.glass_config.name));
      table->setItem(r, 1, it);
    }
  }
}
