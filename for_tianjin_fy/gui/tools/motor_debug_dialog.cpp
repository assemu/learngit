#include "motor_debug_dialog.h"
#include "ui_motor_debug_dialog.h"
#include <gui/util/message_box_helper.h>
#include <core/plc/plc_client.h>
#include <QApplication>
#include <boost/timer/timer.hpp>

const QEvent::Type PLC_POS_CHANGED_EVENT_TYPE = (QEvent::Type)5501;

MotorDebugDialog::MotorDebugDialog()
  : QDialog(), _ui(new Ui::MotorDebugDialog)
{
  _ui->setupUi(this);
  load();

  resize(800, 200);
}

MotorDebugDialog::~MotorDebugDialog()
{
  //删除订阅
  auto c = PLCClient::get();
  c->delNotificationCallBack("GVL_Axis.edge_device_one_physical_location");
  c->delNotificationCallBack("GVL_Axis.edge_device_three_physical_location");
  c->delNotificationCallBack("GVL_Axis.Conveyer_Actpos");
}

void MotorDebugDialog::load()
{
  QStringList v_items = {"5", "15", "25", "35", "50"};
  _ui->comboBox3->addItems(v_items);
  _ui->comboBox4->addItems(v_items);
  _ui->comboBoxBelt->addItems(v_items);

  auto c = PLCClient::get();
  if(!c->isConnect()) {
    log_error("MotorDebugDialog plc is not connected");
    return;
  }

  //读取电机速度值,和下拉框动作响应
  double v3 = 0, v4 = 0, vbelt = 0;
  c->readVar<double>("GVL_Axis.McAbsolute_edge_device_one_velocity", v3);
  c->readVar<double>("GVL_Axis.McAbsolute_edge_device_three_velocity", v4);
  c->readVar<double>("GVL_Axis.Conveyer_Manual_velocity", vbelt);
  _ui->comboBox3->setCurrentText(QString("%1").arg((int)v3));
  _ui->comboBox4->setCurrentText(QString("%1").arg((int)v4));
  _ui->comboBoxBelt->setCurrentText(QString("%1").arg((int)vbelt));
  connect(_ui->comboBox3, &QComboBox::currentTextChanged, this,  [](const QString & text) {
    PLCClient::get()->writeVar<double>("GVL_Axis.McAbsolute_edge_device_one_velocity", text.toDouble());
  });
  connect(_ui->comboBox4, &QComboBox::currentTextChanged, this,  [](const QString & text) {
    PLCClient::get()->writeVar<double>("GVL_Axis.McAbsolute_edge_device_three_velocity", text.toDouble());
  });
  connect(_ui->comboBoxBelt, &QComboBox::currentTextChanged, this,  [](const QString & text) {
    PLCClient::get()->writeVar<double>("GVL_Axis.Conveyer_Manual_velocity", text.toDouble());
  });

  //手动模式加载和动作响应
  bool is_manual = false;
  c->readVar<bool>("GVL_Axis.Hand_Self_Conversion", is_manual);
  _ui->manualCheckBox->setChecked(is_manual);
  connect(_ui->manualCheckBox, &QCheckBox::toggled, this, [](bool checked) {
    PLCClient::get()->writeVar<bool>("GVL_Axis.Hand_Self_Conversion", checked);
  });

  //电机当前位置
  c->setNotificationCallBack("GVL_Axis.edge_device_one_physical_location", 20, sizeof(double), [this](void* data) {
    _pos3 = *((double*)data);
    QCoreApplication::postEvent(this, new QEvent(PLC_POS_CHANGED_EVENT_TYPE));
  });
  c->setNotificationCallBack("GVL_Axis.edge_device_three_physical_location", 20, sizeof(double), [this](void* data) {
    _pos4 = *((double*)data);
    QCoreApplication::postEvent(this, new QEvent(PLC_POS_CHANGED_EVENT_TYPE));
  });
  c->setNotificationCallBack("GVL_Axis.Conveyer_Actpos", 20, sizeof(double), [this](void* data) {
    _pos_belt = *((double*)data);
    QCoreApplication::postEvent(this, new QEvent(PLC_POS_CHANGED_EVENT_TYPE));
  });

  //按钮的动作响应
  connect(_ui->upBtn3, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_one_up", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_one_up", false);
  });

  connect(_ui->upBtn4, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_three_up", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_three_up", false);
  });

  connect(_ui->upBtnBelt, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.Conveyer_Forward", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.Conveyer_Forward", false);
  });

  connect(_ui->downBtn3, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_one_down", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_one_down", false);
  });

  connect(_ui->downBtn4, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_three_down", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.McAbsolute_edge_device_three_down", false);
  });

  connect(_ui->downBtnBelt, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.Conveyer_Backward", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.Conveyer_Backward", false);
  });

  connect(_ui->stopBtn3, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.Mc_Stop_edge_device_one", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.Mc_Stop_edge_device_one", false);
  });

  connect(_ui->stopBtn4, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.Mc_Stop_edge_device_three", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.Mc_Stop_edge_device_three", false);
  });

  connect(_ui->stopBtnBelt, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_Axis.Conveyer_Stop", true);
    _spin(100);
    PLCClient::get()->writeVar<bool>("GVL_Axis.Conveyer_Stop", false);
  });
}

bool MotorDebugDialog::event(QEvent* event)
{
  if(event->type() == QEvent::Wheel)
    return false;

  if(event->type() == PLC_POS_CHANGED_EVENT_TYPE) {
    _ui->posEdit3->setText(QString("%1").arg(_pos3, 0, 'f', 2));
    _ui->posEdit4->setText(QString("%1").arg(_pos4, 0, 'f', 2));
    _ui->posEditBelt->setText(QString("%1").arg(_pos_belt, 0, 'f', 2));
    return true;
  }

  return QWidget::event(event);
}

void MotorDebugDialog::_spin(int ms)
{
  boost::timer::cpu_timer t;
  while(t.elapsed().wall < ms * 1e6) {
    qApp->processEvents();
  }
}
