#include "batch_select_dialog.h"
#include "batch_create_dialog.h"
#include "ui_batch_select_dialog.h"
#include <gui/util/message_box_helper.h>
#include <core/db/db.h>

BatchSelectDialog::BatchSelectDialog()
  : QDialog(), _ui(new Ui::BatchSelectDialog)
{
  _ui->setupUi(this);
  reload();

  connect(_ui->createBtn, &QPushButton::clicked, this,
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

  connect(_ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    this->done(1);
  });

  connect(_ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    this->done(0);
  });
}

QString BatchSelectDialog::getBatchId()
{
  return _ui->selectBox->currentText().trimmed();
}

void BatchSelectDialog::reload()
{
  auto list = DB::get()->queryAllBatches();
  _ui->selectBox->clear();
  for(const auto& b : list) {
    _ui->selectBox->addItem(QString::fromStdString(b.id));
  }
}
