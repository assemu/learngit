#include "plc_movement_setting_widget.h"
#include "ui_plc_movement_widget.h"
#include <QtWidgets>
#include <gui/util/message_box_helper.h>
#include <gui/util/line_chart_widget.h>
#include <core/plc/plc_client.h>
#include <core/plc/plc_util.h>
#include <core/app_config.h>
#include <map>
#include <sstream>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QList>
#include <QStringList>

const QEvent::Type PLC_CAM3_TULUN_SUCCESS_EVENT_TYPE = (QEvent::Type)5331;
const QEvent::Type PLC_CAM3_TULUN_FAILED_EVENT_TYPE = (QEvent::Type)5332;
const QEvent::Type PLC_CAM4_TULUN_SUCCESS_EVENT_TYPE = (QEvent::Type)5341;
const QEvent::Type PLC_CAM4_TULUN_FAILED_EVENT_TYPE = (QEvent::Type)5342;

std::vector<double> readCSVFile(const std::string& file_path)
{
  QFile file(QString::fromStdString(file_path));
  if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    log_error("open csv file {} failed!", file_path);
    return std::vector<double>();
  }

  //加载数据,使用QList提高性能
  QList<QPair<double, double>> dd;
  QTextStream stream(&file);
  while(!stream.atEnd()) {
    QString line = stream.readLine();
    QStringList list = line.split(",");
    if(list.size() >= 2) //每行取前两个数据
      dd.append(qMakePair(list[0].toDouble(), list[1].toDouble()));
  }
  file.close();

  //将所有数据连成一个一维数组
  std::vector<double> re(dd.size() * 2);
  int i = 0;
  for(auto& p : dd) {
    re[2 * i] = p.first;
    re[2 * i + 1] = p.second;
    i++;
  }

  return re;
}

void loadChart(LineChartWidget* chart, const std::vector<double>& data)
{
  int N = data.size() / 2;
  Curve2d c(N);
  for(int i = 0; i < N; i++)
    c.set(i, data[2 * i], data[2 * i + 1]);
  chart->clearAll();
  chart->addCurve(c, Qt::green, 2);
}

PLCMovementSettingWidget::PLCMovementSettingWidget()
  : QWidget(), _ui(new Ui::PLCMovementWidget())
{
  _ui->setupUi(this);

  {
    _chart3 = new LineChartWidget();
    auto ly = new QVBoxLayout();
    ly->setContentsMargins(0, 0, 0, 0);
    ly->addWidget(_chart3);
    _ui->curve_widget_3->setLayout(ly);
  }

  {
    _chart4 = new LineChartWidget();
    auto ly = new QVBoxLayout();
    ly->setContentsMargins(0, 0, 0, 0);
    ly->addWidget(_chart4);
    _ui->curve_widget_4->setLayout(ly);
  }

  connect(_ui->cam3_csv_choose_btn, &QPushButton::clicked, this,
  [this]() {
    QString file_path = QFileDialog::getOpenFileName(this, tr("选择csv文件"),
                        QDir::homePath(),
                        tr("CSV File (*.csv)")).trimmed();
    _cam3_data = readCSVFile(file_path.toStdString());
    loadChart(_chart3, _cam3_data);
  });

  connect(_ui->cam4_csv_choose_btn, &QPushButton::clicked, this,
  [this]() {
    QString file_path = QFileDialog::getOpenFileName(this, tr("选择csv文件"),
                        QDir::homePath(),
                        tr("CSV File (*.csv)")).trimmed();
    _cam4_data = readCSVFile(file_path.toStdString());
    loadChart(_chart4, _cam4_data);
  });

  connect(_ui->down_btn, &QPushButton::clicked, this,
  [this]() {
    if(!PLCClient::get()->isConnect()) {
      std::stringstream ss;
      ss << "PLC没有连接，当前连接地址 " << AppConfig::get()->getPLCID() << ":" << AppConfig::get()->getPLCPort();
      MessageBoxHelper::show_critical(QString::fromStdString(ss.str()));
    } else {
      //创建电子凸轮表是否成功,CAM3
      PLCClient::get()->delNotificationCallBack("GVL_Read_State.edge_device_one_CamTableSelect");
      PLCClient::get()->setNotificationCallBack("GVL_Read_State.edge_device_one_CamTableSelect", 100, 2,
      [this](void* data) {
        int flag = *((int*)data);
        if(flag == 0)
          QCoreApplication::postEvent(this, new QEvent(PLC_CAM3_TULUN_FAILED_EVENT_TYPE));
        else
          QCoreApplication::postEvent(this, new QEvent(PLC_CAM3_TULUN_SUCCESS_EVENT_TYPE));
      });

      //创建电子凸轮表是否成功 CAM4
      PLCClient::get()->delNotificationCallBack("GVL_Read_State.edge_device_three_CamTableSelect");
      PLCClient::get()->setNotificationCallBack("GVL_Read_State.edge_device_three_CamTableSelect", 100, 2,
      [this](void* data) {
        int flag = *((int*)data);
        if(flag == 0)
          QCoreApplication::postEvent(this, new QEvent(PLC_CAM4_TULUN_FAILED_EVENT_TYPE));
        else
          QCoreApplication::postEvent(this, new QEvent(PLC_CAM4_TULUN_SUCCESS_EVENT_TYPE));
      });

      PlcConfigTrack c;
      getConfig(c);
      if(sendPlcConfigTrack(c)) {
        MessageBoxHelper::show_info("写入成功！");
      } else {
        MessageBoxHelper::show_info("写入失败！");
      }
    }
  });

  resize(1100, 600);
}

PLCMovementSettingWidget::~PLCMovementSettingWidget()
{
  PLCClient::get()->delNotificationCallBack("GVL_Read_State.edge_device_one_CamTableSelect");
  PLCClient::get()->delNotificationCallBack("GVL_Read_State.edge_device_three_CamTableSelect");
}

bool PLCMovementSettingWidget::event(QEvent* event)
{
  if(event->type() == PLC_CAM3_TULUN_SUCCESS_EVENT_TYPE) {
    _ui->cam3_success_btn->setIcon(QIcon(":icons/OK.png"));
    return true;
  }

  if(event->type() == PLC_CAM3_TULUN_FAILED_EVENT_TYPE) {
    _ui->cam3_success_btn->setIcon(QIcon(":icons/NOK.png"));
    return true;
  }

  if(event->type() == PLC_CAM4_TULUN_SUCCESS_EVENT_TYPE) {
    _ui->cam4_success_btn->setIcon(QIcon(":icons/OK.png"));
    return true;
  }

  if(event->type() == PLC_CAM4_TULUN_FAILED_EVENT_TYPE) {
    _ui->cam4_success_btn->setIcon(QIcon(":icons/NOK.png"));
    return true;
  }

  return QWidget::event(event);
}

void PLCMovementSettingWidget::setConfig(const PlcConfigTrack& c)
{
  {
    bool v = 0;
    if(c.getConfigItem<bool>("GVL_Read_State.Model_Change_Table", v))
      _ui->true_mode_checkbox->setChecked(v);
  }

  {
    double v = 0;
    if(c.getConfigItem<double>("GVL_Read_State.WriteParameter_edge_device_one_value", v))
      _ui->edge_device_three_value_edit->setValue(v);
  }

  {
    double v = 0;
    if(c.getConfigItem<double>("GVL_Read_State.WriteParameter_edge_device_three_value", v))
      _ui->edge_device_four_value_edit->setValue(v);
  }

  {
    if(c.getCAMTrackData("GVL_Read_State.coordinate_one", _cam3_data)) {
      loadChart(_chart3, _cam3_data);
    }
  }

  {
    if(c.getCAMTrackData("GVL_Read_State.coordinate_three", _cam4_data)) {
      loadChart(_chart4, _cam4_data);
    }
  }
}

void PLCMovementSettingWidget::getConfig(PlcConfigTrack& c)
{
  c.setConfigItem<bool>("GVL_Read_State.Model_Change_Table", _ui->true_mode_checkbox->isChecked());
  c.setConfigItem<double>("GVL_Read_State.WriteParameter_edge_device_one_value",  _ui->edge_device_three_value_edit->value());
  c.setConfigItem<double>("GVL_Read_State.WriteParameter_edge_device_three_value",  _ui->edge_device_four_value_edit->value());
  c.setCAMTrackData("GVL_Read_State.coordinate_one", _cam3_data);
  c.setCAMTrackData("GVL_Read_State.coordinate_three", _cam4_data);
}
