#include "face_cam_thresholds_widget.h"
#include "ui_face_cam_thresholds_widget.h"

FaceCAMThresholdsWidget::FaceCAMThresholdsWidget()
  : QWidget(), _ui(new Ui::FaceCAMThresholdsWidget)
{
  _ui->setupUi(this);

  QList<QDoubleSpinBox*> edits;
  edits << _ui->binary_threshold_edit
        << _ui->bright_edit << _ui->bright_area_edit << _ui->bright_length_edit
        << _ui->dark_edit << _ui->dark_area_edit << _ui->dark_length_edit
        << _ui->dirty_edit << _ui->dirty_area_edit << _ui->dirty_length_edit
        << _ui->scratch_edit << _ui->scratch_area_edit << _ui->scratch_length_edit
        << _ui->circle_edit << _ui->circle_area_edit << _ui->circle_length_edit;

  for(auto& edit : edits) {
    connect(edit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    [this](double d) {
      emit paramChanged();
    });
  }
}

FaceCAMThresholdsWidget::~FaceCAMThresholdsWidget()
{
  delete _ui;
}

void FaceCAMThresholdsWidget::setFaceImgSplitBinaryThreshhold(float v)
{
  _ui->binary_threshold_edit->setValue(v);
}

float FaceCAMThresholdsWidget::getFaceImgSplitBinaryThreshhold()
{
  return _ui->binary_threshold_edit->value();
}

void FaceCAMThresholdsWidget::setConfidenceProbability(const std::string& label, float p)
{
  if(label == "bright") {
    _ui->bright_edit->setValue(p);
  }

  if(label == "dark") {
    _ui->dark_edit->setValue(p);
  }

  if(label == "dirty") {
    _ui->dirty_edit->setValue(p);
  }

  if(label == "scratch") {
    _ui->scratch_edit->setValue(p);
  }

  if(label == "circle") {
    _ui->circle_edit->setValue(p);
  }
}

void FaceCAMThresholdsWidget::setAreaThreshold(const std::string& label, float p)
{
  if(label == "bright") {
    _ui->bright_area_edit->setValue(p);
  }

  if(label == "dark") {
    _ui->dark_area_edit->setValue(p);
  }

  if(label == "dirty") {
    _ui->dirty_area_edit->setValue(p);
  }

  if(label == "scratch") {
    _ui->scratch_area_edit->setValue(p);
  }

  if(label == "circle") {
    _ui->circle_area_edit->setValue(p);
  }
}

void FaceCAMThresholdsWidget::setLengthThreshold(const std::string& label, float p)
{
  if(label == "bright") {
    _ui->bright_length_edit->setValue(p);
  }

  if(label == "dark") {
    _ui->dark_length_edit->setValue(p);
  }

  if(label == "dirty") {
    _ui->dirty_length_edit->setValue(p);
  }

  if(label == "scratch") {
    _ui->scratch_length_edit->setValue(p);
  }

  if(label == "circle") {
    _ui->circle_length_edit->setValue(p);
  }
}

float FaceCAMThresholdsWidget::getConfidenceProbability(const std::string& label)
{
  float re = 0.0f;

  if(label == "bright") {
    re = _ui->bright_edit->value();
  }

  if(label == "dark") {
    re = _ui->dark_edit->value();
  }

  if(label == "dirty") {
    re = _ui->dirty_edit->value();
  }

  if(label == "scratch") {
    re = _ui->scratch_edit->value();
  }

  if(label == "circle") {
    re = _ui->circle_edit->value();
  }

  return re;
}

float FaceCAMThresholdsWidget::getAreaThreshold(const std::string& label)
{
  float re = 0.0f;

  if(label == "bright") {
    re = _ui->bright_area_edit->value();
  }

  if(label == "dark") {
    re = _ui->dark_area_edit->value();
  }

  if(label == "dirty") {
    re = _ui->dirty_area_edit->value();
  }

  if(label == "scratch") {
    re = _ui->scratch_area_edit->value();
  }

  if(label == "circle") {
    re = _ui->circle_area_edit->value();
  }

  return re;
}

float FaceCAMThresholdsWidget::getLengthThreshold(const std::string& label)
{
  float re = 0.0f;

  if(label == "bright") {
    re = _ui->bright_length_edit->value();
  }

  if(label == "dark") {
    re = _ui->dark_length_edit->value();
  }

  if(label == "dirty") {
    re = _ui->dirty_length_edit->value();
  }

  if(label == "scratch") {
    re = _ui->scratch_length_edit->value();
  }

  if(label == "circle") {
    re = _ui->circle_length_edit->value();
  }

  return re;
}

void FaceCAMThresholdsWidget::setLabelFilterOut(const std::string& label, bool p)
{
  if(label == "bright") {
    _ui->bright_checkbox->setChecked(!p);
  }

  if(label == "dark") {
    _ui->dark_checkbox->setChecked(!p);
  }

  if(label == "dirty") {
    _ui->dirty_checkbox->setChecked(!p);
  }

  if(label == "scratch") {
    _ui->scratch_checkbox->setChecked(!p);
  }

  if(label == "circle") {
    _ui->circle_checkbox->setChecked(!p);
  }
}

bool FaceCAMThresholdsWidget::getLabelFilterOut(const std::string& label)
{
  bool re = false;

  if(label == "bright") {
    re = !_ui->bright_checkbox->isChecked();
  }

  if(label == "dark") {
    re = !_ui->dark_checkbox->isChecked();
  }

  if(label == "dirty") {
    re = !_ui->dirty_checkbox->isChecked();
  }

  if(label == "scratch") {
    re = !_ui->scratch_checkbox->isChecked();
  }

  if(label == "circle") {
    re = !_ui->circle_checkbox->isChecked();
  }

  return re;
}
