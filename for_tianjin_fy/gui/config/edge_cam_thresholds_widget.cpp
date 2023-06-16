#include "edge_cam_thresholds_widget.h"
#include "ui_edge_cam_thresholds_widget.h"

EdgeCAMThresholdsWidget::EdgeCAMThresholdsWidget()
  : QWidget(), _ui(new Ui::EdgeCAMThresholdsWidget)
{
  _ui->setupUi(this);

  QList<QDoubleSpinBox*> edits;
  edits << _ui->white_edit
        << _ui->white_area_edit
        << _ui->white_length_edit
        << _ui->black_edit
        << _ui->black_area_edit
        << _ui->black_length_edit;

  for(auto& edit : edits) {
    connect(edit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    [this](double d) {
      emit paramChanged();
    });
  }
}

EdgeCAMThresholdsWidget::~EdgeCAMThresholdsWidget()
{
  delete _ui;
}

void EdgeCAMThresholdsWidget::setConfidenceProbability(const std::string& label, float p)
{
  if(label == "white") {
    _ui->white_edit->setValue(p);
  }

  if(label == "black") {
    _ui->black_edit->setValue(p);
  }
}

void EdgeCAMThresholdsWidget::setAreaThreshold(const std::string& label, float p)
{
  if(label == "white") {
    _ui->white_area_edit->setValue(p);
  }

  if(label == "black") {
    _ui->black_area_edit->setValue(p);
  }
}

void EdgeCAMThresholdsWidget::setLengthThreshold(const std::string& label, float p)
{
  if(label == "white") {
    _ui->white_length_edit->setValue(p);
  }

  if(label == "black") {
    _ui->black_length_edit->setValue(p);
  }
}

float EdgeCAMThresholdsWidget::getConfidenceProbability(const std::string& label)
{
  float re = 0.0f;

  if(label == "white") {
    re = _ui->white_edit->value();
  }

  if(label == "black") {
    re = _ui->black_edit->value();
  }

  return re;
}

float EdgeCAMThresholdsWidget::getAreaThreshold(const std::string& label)
{
  float re = 0.0f;

  if(label == "white") {
    re = _ui->white_area_edit->value();
  }

  if(label == "black") {
    re = _ui->black_area_edit->value();
  }

  return re;
}

float EdgeCAMThresholdsWidget::getLengthThreshold(const std::string& label)
{
  float re = 0.0f;

  if(label == "white") {
    re = _ui->white_length_edit->value();
  }

  if(label == "black") {
    re = _ui->black_length_edit->value();
  }

  return re;
}

void EdgeCAMThresholdsWidget::setLabelFilterOut(const std::string& label, bool p)
{
  if(label == "white") {
    _ui->white_checkbox->setChecked(!p);
  }

  if(label == "black") {
    _ui->black_checkbox->setChecked(!p);
  }
}

bool EdgeCAMThresholdsWidget::getLabelFilterOut(const std::string& label)
{
  bool re = false;

  if(label == "white") {
    re = !_ui->white_checkbox->isChecked();
  }

  if(label == "black") {
    re = !_ui->black_checkbox->isChecked();
  }

  return re;
}
