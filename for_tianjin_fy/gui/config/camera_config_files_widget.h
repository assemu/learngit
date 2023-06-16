#ifndef GUI_CAMERA_CONFIG_FILES_WIDGET_H_
#define GUI_CAMERA_CONFIG_FILES_WIDGET_H_

#include <QWidget>
#include <core/common.h>

namespace Ui
{
class CameraConfigFilesWidget;
}

class CameraConfigFilesWidget : public QWidget
{
  Q_OBJECT;
public:
  CameraConfigFilesWidget();
  ~CameraConfigFilesWidget();

  void enableEditName();
  void disableEditName();
  std::string getName();
  void setName(const std::string& name);
  std::map<CameraPosition, std::string> getCameraConfigFiles();
  void setCameraConfigFiles(const std::map<CameraPosition, std::string>& c);

signals:
  void paramChanged();

private:
  Ui::CameraConfigFilesWidget* _ui;
};

#endif //GUI_CAMERA_CONFIG_FILES_WIDGET_H_
