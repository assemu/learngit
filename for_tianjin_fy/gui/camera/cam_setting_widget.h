#ifndef BIOCONF_GUI_CAM_SETTING_WIDGET_H_
#define BIOCONF_GUI_CAM_SETTING_WIDGET_H_

#include <QWidget>
#include <core/common.h>

namespace Ui
{
class CamSettingWidget;
}

class CamSettingWidget : public QWidget
{
  Q_OBJECT;
public:
  CamSettingWidget(CameraPosition position);
  ~CamSettingWidget();

private:
  CameraPosition _position;
  Ui::CamSettingWidget* _ui;
};

#endif //BIOCONF_GUI_CAM_SETTING_WIDGET_H_
