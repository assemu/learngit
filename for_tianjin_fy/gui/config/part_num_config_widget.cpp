#include "part_num_config_widget.h"
#include "ui_part_num_config_widget.h"

PartNumConfigWidget::PartNumConfigWidget()
  : QWidget(), _ui(new Ui::PartNumConfigWidget)
{
  _ui->setupUi(this);

  QList<QSpinBox*> edits;
  edits << _ui->cam3_begin_edit << _ui->cam3_end_edit << _ui->cam3_steps_edit;
  edits << _ui->cam4_begin_edit << _ui->cam4_end_edit << _ui->cam4_steps_edit;
  edits << _ui->cam5_begin_edit << _ui->cam5_end_edit << _ui->cam5_steps_edit;
  edits << _ui->cam6_begin_edit << _ui->cam6_end_edit << _ui->cam6_steps_edit;
  edits << _ui->cam7_begin_edit << _ui->cam7_end_edit << _ui->cam7_steps_edit;
  edits << _ui->cam8_begin_edit << _ui->cam8_end_edit << _ui->cam8_steps_edit;
  edits << _ui->cam1_begin_edit << _ui->cam1_end_edit << _ui->cam1_steps_edit;
  edits << _ui->cam2_begin_edit << _ui->cam2_end_edit << _ui->cam2_steps_edit;

  for(auto& edit : edits) {
    connect(edit, QOverload<int>::of(&QSpinBox::valueChanged),
    [this](int d) {
      emit paramChanged();
    });
  }
}

PartNumConfigWidget::~PartNumConfigWidget()
{
  delete _ui;
}

std::map<CameraPosition, int> PartNumConfigWidget::getBeginSteps()
{
  std::map<CameraPosition, int> re;
  re[EdgeRight] = _ui->cam3_begin_edit->value();
  re[EdgeLeft] = _ui->cam4_begin_edit->value();
  re[FrontRight] = _ui->cam5_begin_edit->value();
  re[FrontLeft] = _ui->cam6_begin_edit->value();
  re[BackRight] = _ui->cam7_begin_edit->value();
  re[BackLeft] = _ui->cam8_begin_edit->value();
  re[TopLeft] = _ui->cam1_begin_edit->value();
  re[TopRight] = _ui->cam2_begin_edit->value();
  return re;
}

void PartNumConfigWidget::setBeginSteps(const std::map<CameraPosition, int>& c)
{
  _ui->cam3_begin_edit->setValue(c.at(EdgeRight));
  _ui->cam4_begin_edit->setValue(c.at(EdgeLeft));
  _ui->cam5_begin_edit->setValue(c.at(FrontRight));
  _ui->cam6_begin_edit->setValue(c.at(FrontLeft));
  _ui->cam7_begin_edit->setValue(c.at(BackRight));
  _ui->cam8_begin_edit->setValue(c.at(BackLeft));
  _ui->cam1_begin_edit->setValue(c.at(TopLeft));
  _ui->cam2_begin_edit->setValue(c.at(TopRight));
}

std::map<CameraPosition, int> PartNumConfigWidget::getEndSteps()
{
  std::map<CameraPosition, int> re;
  re[EdgeRight] = _ui->cam3_end_edit->value();
  re[EdgeLeft] = _ui->cam4_end_edit->value();
  re[FrontRight] = _ui->cam5_end_edit->value();
  re[FrontLeft] = _ui->cam6_end_edit->value();
  re[BackRight] = _ui->cam7_end_edit->value();
  re[BackLeft] = _ui->cam8_end_edit->value();
  re[TopLeft] = _ui->cam1_end_edit->value();
  re[TopRight] = _ui->cam2_end_edit->value();
  return re;
}

void PartNumConfigWidget::setEndSteps(const std::map<CameraPosition, int>& c)
{
  _ui->cam3_end_edit->setValue(c.at(EdgeRight));
  _ui->cam4_end_edit->setValue(c.at(EdgeLeft));
  _ui->cam5_end_edit->setValue(c.at(FrontRight));
  _ui->cam6_end_edit->setValue(c.at(FrontLeft));
  _ui->cam7_end_edit->setValue(c.at(BackRight));
  _ui->cam8_end_edit->setValue(c.at(BackLeft));
  _ui->cam1_end_edit->setValue(c.at(TopLeft));
  _ui->cam2_end_edit->setValue(c.at(TopRight));
}

std::map<CameraPosition, int> PartNumConfigWidget::getSteps()
{
  std::map<CameraPosition, int> re;
  re[EdgeRight] = _ui->cam3_steps_edit->value();
  re[EdgeLeft] = _ui->cam4_steps_edit->value();
  re[FrontRight] = _ui->cam5_steps_edit->value();
  re[FrontLeft] = _ui->cam6_steps_edit->value();
  re[BackRight] = _ui->cam7_steps_edit->value();
  re[BackLeft] = _ui->cam8_steps_edit->value();
  re[TopLeft] = _ui->cam1_steps_edit->value();
  re[TopRight] = _ui->cam2_steps_edit->value();
  return re;
}

void PartNumConfigWidget::setSteps(const std::map<CameraPosition, int>& c)
{
  _ui->cam3_steps_edit->setValue(c.at(EdgeRight));
  _ui->cam4_steps_edit->setValue(c.at(EdgeLeft));
  _ui->cam5_steps_edit->setValue(c.at(FrontRight));
  _ui->cam6_steps_edit->setValue(c.at(FrontLeft));
  _ui->cam7_steps_edit->setValue(c.at(BackRight));
  _ui->cam8_steps_edit->setValue(c.at(BackLeft));
  _ui->cam1_steps_edit->setValue(c.at(TopLeft));
  _ui->cam2_steps_edit->setValue(c.at(TopRight));
}
