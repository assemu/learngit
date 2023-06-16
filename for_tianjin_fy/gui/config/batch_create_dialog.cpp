#include "batch_create_dialog.h"
#include "ui_batch_create_dialog.h"
#include <gui/util/message_box_helper.h>
#include <core/db/db.h>

BatchCreateDialog::BatchCreateDialog()
  : QDialog(), _ui(new Ui::BatchCreateDialog)
{
  _ui->setupUi(this);

  auto list = DB::get()->queryAllGlassConfigNames();
  for(auto& l : list)
    _ui->selectBox->addItem(QString::fromStdString(l));

  connect(_ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    if(getBatchId().isEmpty()) {
      MessageBoxHelper::show_critical("批号ID不能为空！");
      return;
    }
    if(getGlassConfigName().isEmpty()) {
      MessageBoxHelper::show_critical("玻璃型号不能为空！");
      return;
    }
    if(DB::get()->isBatchExist(getBatchId().toStdString())) {
      MessageBoxHelper::show_critical("批号ID已经存在！");
      return;
    }

    this->done(1);
  });

  connect(_ui->cancelBtn, &QPushButton::clicked, this,
  [this]() {
    this->done(0);
  });
}

QString BatchCreateDialog::getBatchId()
{
  return _ui->id_edit->text().trimmed();
}

QString BatchCreateDialog::getGlassConfigName()
{
  return _ui->selectBox->currentText().trimmed();
}
