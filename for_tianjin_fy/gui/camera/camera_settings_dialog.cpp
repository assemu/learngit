#include "camera_settings_dialog.h"
#include "cam_setting_widget.h"
#include <QtWidgets>
#include <gui/util/message_box_helper.h>
#include <QDateTime>

CameraSettingsDialog::CameraSettingsDialog()
  : QDialog()
{
  auto ly = new QGridLayout();

  auto ps = allCameraPositions();
  for(int i = 0; i < ps.size(); i++) {
    auto position = ps[i];
    auto gw = new QGroupBox(QString("%1 参数设置").arg(QString::fromStdString(cameraNameOf(position))));
    auto cw = new CamSettingWidget(position);
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(cw);
    vbox->setContentsMargins(0, 0, 0, 0);
    gw->setLayout(vbox);
    ly->addWidget(gw, i / 3, i % 3);
  }

  setLayout(ly);
  resize(900, 500);
}

CameraSettingsDialog::~CameraSettingsDialog()
{
}
