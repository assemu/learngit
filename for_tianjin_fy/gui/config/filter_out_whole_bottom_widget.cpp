#include "filter_out_whole_bottom_widget.h"
#include "ui_filter_out_whole_bottom_widget.h"

FilterOutWholeBottomWidget::FilterOutWholeBottomWidget()
  : QWidget(), _ui(new Ui::FilterOutWholeBottomWidget)
{
  _ui->setupUi(this);

  //connect(_ui->checkBox, &QCheckBox::toggled, this,
  //[this](bool on) {
  //});
}

void FilterOutWholeBottomWidget::loadParam(const GlassConfig& c)
{
  _ui->checkBox->setChecked(c.filter_out_whole_bottom);
}

void FilterOutWholeBottomWidget::getParam(GlassConfig& c)
{
  c.filter_out_whole_bottom = _ui->checkBox->isChecked();
}
