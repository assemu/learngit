#include "cam_setting_widget.h"
#include "ui_cam_setting_widget.h"
#include <QtWidgets>
#include <gui/util/message_box_helper.h>
#include <QDateTime>

#ifndef MOCK_RUN
#include <core/camera/camera_part_img_source.h>
#endif

CamSettingWidget::CamSettingWidget(CameraPosition position)
  : QWidget(), _ui(new Ui::CamSettingWidget), _position(position)
{
#ifndef MOCK_RUN
  _ui->setupUi(this);
  auto cs = CameraPartImgSource::get();

  //TODO 考虑读取和设置失败的逻辑
  //{
  //  //读取参数最大值和最小值并设置到控件中
  //  int vmin = 0;
  //  int vmax = 0;
  //  cs->GetParameterTriggerDelayMin(_position, &vmin);
  //  cs->GetParameterTriggerDelayMax(_position, &vmax);
  //  _ui->cam_delay_edit->setMinimum(vmin);
  //  _ui->cam_delay_edit->setMaximum(vmax);

  //  //读取参数值并设置的控件中
  //  int triggerDelay = 0;
  //  cs->GetParameterTriggerDelay(_position, &triggerDelay);
  //  _ui->cam_delay_edit->setValue(triggerDelay);

  //  //值改变的响应逻辑
  //  connect(_ui->cam_delay_edit, QOverload<int>::of(&QSpinBox::valueChanged),
  //  [this, vmin, vmax](int d) {
  //    if(d <= vmax && d >= vmin) {
  //      auto cs = CameraPartImgSource::get();
  //      cs->SetParameterTriggerDelay(_position, d); //设置到相机
  //    }
  //  });
  //}

  /// FredrickLee 20221203
  //曝光时间
  double ExposureTime = 0.0;
  cs->GetFeatureExposureTime(_position, &ExposureTime);
  _ui->cam_delay_edit->setValue(ExposureTime);
  // 值修改响应
  connect(_ui->cam_delay_edit, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
  [this](double d) {
    auto cs = CameraPartImgSource::get();
    //cs->SetFeatureExposureTime(_position, d); //设置到相机

  });

  //单帧行数
  int LineCount = 0;
  cs->GetFeatureLineCount(_position, &LineCount);
  _ui->cam_x_edit_2->setValue(LineCount);

  //connect(_ui->cam_x_edit_2, QOverload<int>::of(&QSpinBox::valueChanged), 	  [this,])
  //帧数
  int FrameCount = 0;
  cs->GetFeatureFrameCount(_position, &FrameCount);
  _ui->cam_x_edit_6->setValue(FrameCount);
  // 值修改响应
  connect(_ui->cam_x_edit_6, QOverload<int>::of(&QSpinBox::valueChanged),
	  [this](int d) {
	  auto cs = CameraPartImgSource::get();
	  //cs->SetFeatureFrameCount(_position, d); //设置到相机

  });

  //行频
  double AcqLineRate = 0.0;
  cs->GetFeatureLineRate(_position, &AcqLineRate);
  _ui->cam_x_edit_3->setValue(AcqLineRate);
  // 值修改响应
  connect(_ui->cam_x_edit_3, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	  [this](double d) {
	  auto cs = CameraPartImgSource::get();
	  //cs->SetFeatureLineRate(_position, d); //设置到相机
  });

  //增益
  double GainValue = 0.0;
  cs->GetFeatureGain(_position, &GainValue);
  _ui->cam_x_edit_4->setValue(GainValue);

  //IO
#endif
}

CamSettingWidget::~CamSettingWidget()
{
  delete _ui;
}
