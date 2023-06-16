#include "camera_config_files_widget.h"
#include "ui_camera_config_files_widget.h"
#include <QtWidgets>

CameraConfigFilesWidget::CameraConfigFilesWidget()
  : QWidget(), _ui(new Ui::CameraConfigFilesWidget)
{
  _ui->setupUi(this);

  QList<QPair<QPushButton*, QLineEdit*>> dd;
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam3_btn, _ui->cam3_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam4_btn, _ui->cam4_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam5_btn, _ui->cam5_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam6_btn, _ui->cam6_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam7_btn, _ui->cam7_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam8_btn, _ui->cam8_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam1_btn, _ui->cam1_file_edit);
  dd << QPair<QPushButton*, QLineEdit*>(_ui->cam2_btn, _ui->cam2_file_edit);

  for(auto& p : dd) {
    auto btn = p.first;
    auto edit = p.second;
    connect(btn, &QPushButton::clicked, this, [btn, edit, this]() {
      QString file_path = QFileDialog::getOpenFileName(NULL, tr("请选择相机配置文件"), ".", tr("相机配置文件(*.ccf)"));
      edit->setText(file_path);
      emit paramChanged();
    });
  }
}

CameraConfigFilesWidget::~CameraConfigFilesWidget()
{
  delete _ui;
}

std::map<CameraPosition, std::string> CameraConfigFilesWidget::getCameraConfigFiles()
{
  std::map<CameraPosition, std::string> re;

  re[EdgeRight] = _ui->cam3_file_edit->text().trimmed().toStdString();
  re[EdgeLeft] = _ui->cam4_file_edit->text().trimmed().toStdString();
  re[FrontRight] = _ui->cam5_file_edit->text().trimmed().toStdString();
  re[FrontLeft] = _ui->cam6_file_edit->text().trimmed().toStdString();
  re[BackRight] = _ui->cam7_file_edit->text().trimmed().toStdString();
  re[BackLeft] = _ui->cam8_file_edit->text().trimmed().toStdString();
  re[TopLeft] = _ui->cam1_file_edit->text().trimmed().toStdString();
  re[TopRight] = _ui->cam2_file_edit->text().trimmed().toStdString();

  return re;
}

void CameraConfigFilesWidget::setCameraConfigFiles(const std::map<CameraPosition, std::string>& c)
{
  _ui->cam3_file_edit->setText(QString::fromStdString(c.at(EdgeRight)));
  _ui->cam4_file_edit->setText(QString::fromStdString(c.at(EdgeLeft)));
  _ui->cam5_file_edit->setText(QString::fromStdString(c.at(FrontRight)));
  _ui->cam6_file_edit->setText(QString::fromStdString(c.at(FrontLeft)));
  _ui->cam7_file_edit->setText(QString::fromStdString(c.at(BackRight)));
  _ui->cam8_file_edit->setText(QString::fromStdString(c.at(BackLeft)));
  _ui->cam1_file_edit->setText(QString::fromStdString(c.at(TopLeft)));
  _ui->cam2_file_edit->setText(QString::fromStdString(c.at(TopRight)));
}

std::string CameraConfigFilesWidget::getName()
{
  return _ui->name_edit->text().trimmed().toStdString();
}

void CameraConfigFilesWidget::setName(const std::string& name)
{
  _ui->name_edit->setText(QString::fromStdString(name));
}

void CameraConfigFilesWidget::enableEditName()
{
  _ui->name_edit->setEnabled(true);
}

void CameraConfigFilesWidget::disableEditName()
{
  _ui->name_edit->setEnabled(false);
}
