#include "roi_config_widget.h"
#include "ui_roi_config_widget.h"

RoiConfigWidget::RoiConfigWidget()
  : QWidget(), _ui(new Ui::RoiConfigWidget)
{
  _ui->setupUi(this);

  QList<QSpinBox*> edits;
  edits << _ui->cam3_x_edit << _ui->cam3_y_edit << _ui->cam3_width_edit << _ui->cam3_height_edit;
  edits << _ui->cam4_x_edit << _ui->cam4_y_edit << _ui->cam4_width_edit << _ui->cam4_height_edit;

  for(auto& edit : edits) {
    connect(edit, QOverload<int>::of(&QSpinBox::valueChanged),
    [this](int d) {
      emit paramChanged();
    });
  }
}

RoiConfigWidget::~RoiConfigWidget()
{
  delete _ui;
}

void RoiConfigWidget::setRoiConfig(const std::map<CameraPosition, cv::Rect>& c)
{
  auto r3 = c.at(EdgeRight);
  auto r4 = c.at(EdgeLeft);

  _ui->cam3_x_edit->setValue(r3.x);
  _ui->cam3_y_edit->setValue(r3.y);
  _ui->cam3_width_edit->setValue(r3.width);
  _ui->cam3_height_edit->setValue(r3.height);

  _ui->cam4_x_edit->setValue(r4.x);
  _ui->cam4_y_edit->setValue(r4.y);
  _ui->cam4_width_edit->setValue(r4.width);
  _ui->cam4_height_edit->setValue(r4.height);
}

std::map<CameraPosition, cv::Rect> RoiConfigWidget::getRoiConfig()
{
  std::map<CameraPosition, cv::Rect> re;
  {
    int x = _ui->cam3_x_edit->value();
    int y = _ui->cam3_y_edit->value();
    int w = _ui->cam3_width_edit->value();
    int h = _ui->cam3_height_edit->value();
    re[EdgeRight] = cv::Rect(x, y, w, h);
  }

  {
    int x = _ui->cam4_x_edit->value();
    int y = _ui->cam4_y_edit->value();
    int w = _ui->cam4_width_edit->value();
    int h = _ui->cam4_height_edit->value();
    re[EdgeLeft] = cv::Rect(x, y, w, h);
  }

  return re;
}
