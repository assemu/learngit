#include "plc_device_run_dialog.h"
#include "ui_plc_device_run_dialog.h"
#include <gui/util/message_box_helper.h>
#include <core/plc/plc_client.h>
#include <QApplication>
#include <vector>
#include <boost/timer/timer.hpp>
#include "circle_label.h"

const QEvent::Type PLC_DEVICE_CHANGED_EVENT_TYPE = (QEvent::Type)5502;

static std::vector<std::string> err_keys = {
  "GVL_Read_State.Axis_conveyer_Error",  //传送带电机错误状态
  "GVL_Read_State.Error_state", //系统错误状态
  "GVL_Read_State.Communication", //通讯指示
  "GVL_Read_State.Axis_edge_device_one_Error", //边缘电机3错误状态
  "GVL_Read_State.Axis_edge_device_three_Error", //边缘电机4错误状态
  "GVL_Axis.edge_device_one_origin", //边缘电机3原点指示
  "GVL_Axis.edge_device_three_origin" //边缘电机4原点指示
};

static std::vector<std::string> err_code_keys = {
  "GVL_Read_State.Axis_conveyer_Errorcode", //传送带电机错误码
  "GVL_Read_State.Axis_edge_device_one_Errorcode", //边缘电机3错误码
  "GVL_Read_State.Axis_edge_device_three_Errorcode"  //边缘电机4错误码
};

PLCDeviceRunDialog::PLCDeviceRunDialog()
  : QDialog(), _ui(new Ui::PLCDeviceRunDialog)
{
  _ui->setupUi(this);

  //key与组件对应关系
  _err_labels["GVL_Read_State.Axis_conveyer_Error"] = _ui->errStatusLabelBelt; //传送带电机错误状态
  _err_labels["GVL_Read_State.Error_state"] = _ui->errStatusLabelSystem; //系统错误状态
  _err_labels["GVL_Read_State.Communication"] = _ui->errStatusLabelCon; //通讯指示
  _err_labels["GVL_Read_State.Axis_edge_device_one_Error"] = _ui->errStatusLabel3; //边缘电机3错误状态
  _err_labels["GVL_Read_State.Axis_edge_device_three_Error"] = _ui->errStatusLabel4; //边缘电机4错误状态
  _err_labels["GVL_Axis.edge_device_one_origin"] = _ui->originLabel3; //边缘电机3原点指示
  _err_labels["GVL_Axis.edge_device_three_origin"] = _ui->originLabel4; //边缘电机4原点指示

  _err_code_labels["GVL_Read_State.Axis_conveyer_Errorcode"] = _ui->errCodeLabelBelt; //传送带电机错误码
  _err_code_labels["GVL_Read_State.Axis_edge_device_one_Errorcode"] = _ui->errCodeLabel3; //边缘电机3错误码
  _err_code_labels["GVL_Read_State.Axis_edge_device_three_Errorcode"] = _ui->errCodeLabel4; //边缘电机4错误码

  for(auto& kv : _err_labels)
    kv.second->setText("");//清空设计是做得标记字符串

  load();

  resize(800, 350);
}

PLCDeviceRunDialog::~PLCDeviceRunDialog()
{
  auto c = PLCClient::get();
  for(auto& key : err_keys) {
    c->delNotificationCallBack(key);
  }
}

void PLCDeviceRunDialog::load()
{
  QStringList v_items = {"75", "196", "300", "350", "400", "500", "319", "320"};
  _ui->beltVecComboBox->addItems(v_items);

  auto c = PLCClient::get();
  if(!c->isConnect()) {
    log_error("PLCDeviceRunDialog plc is not connected");
    return;
  }

  //读取传送带电机自动速度值
  double automatic_velocity = 0, pos_front = 0, pos_profile = 0;
  c->readVar<double>("GVL_Axis.Conveyer_Automatic_velocity", automatic_velocity);
  c->readVar<double>("GVL_digit_group.position_front", pos_front);
  c->readVar<double>("GVL_digit_group.position_profile", pos_profile);
  _ui->beltVecComboBox->setCurrentText(QString("%1").arg((int)automatic_velocity));
  _ui->frontTriggerPosEdit->setText(QString("%1").arg(pos_front, 0, 'f', 2));
  _ui->edgeTriggerPosEdit->setText(QString("%1").arg(pos_profile, 0, 'f', 2));

  //圆圈
  for(auto& key : err_keys) {
    c->setNotificationCallBack(key, 200, sizeof(bool), [key, this](void* data) {
      _err_map[key] = *((bool*)data);
      //一种方式可以使得更新界面的动作在界面主线程中运行
      QCoreApplication::postEvent(this, new QEvent(PLC_DEVICE_CHANGED_EVENT_TYPE));
    });
  }
  //错误码
  for(auto& key : err_code_keys) {
    c->setNotificationCallBack(key, 200, sizeof(int), [key, this](void* data) {
      _err_code_map[key] = *((int*)data);
      QCoreApplication::postEvent(this, new QEvent(PLC_DEVICE_CHANGED_EVENT_TYPE));
    });
  }

  //下拉框,响应
  connect(_ui->beltVecComboBox, &QComboBox::currentTextChanged, this,  [](const QString & text) {
    PLCClient::get()->writeVar<double>("GVL_Axis.Conveyer_Automatic_velocity", text.toDouble());
  });

  //按钮的动作响应
  connect(_ui->forwardBtn, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_IO.manual_conveyer_belt", true);
  });
  connect(_ui->stopBtn, &QPushButton::clicked, this, [this]() {
    PLCClient::get()->writeVar<bool>("GVL_IO.manual_conveyer_belt", false);
  });
}

bool PLCDeviceRunDialog::event(QEvent* event)
{
  if(event->type() == QEvent::Wheel)
    return false;

  //更新界面(界面主线程中执行)
  if(event->type() == PLC_DEVICE_CHANGED_EVENT_TYPE) {
    for(auto& key : err_keys) {
      if(_err_map.count(key) > 0 && _err_labels.count(key) > 0) {
        bool v = _err_map[key];
        if(key == "GVL_Read_State.Communication") //通信指示true代表绿色
          v = !v;
        CircleLabel* l = _err_labels[key];
        if(v == true)
          l->setColor(Qt::red);
        else
          l->setColor(Qt::green);
      }
    }

    for(auto& key : err_code_keys) {
      if(_err_code_map.count(key) > 0 && _err_code_labels.count(key) > 0) {
        int v = _err_code_map[key];
        QLabel* l = _err_code_labels[key];
        l->setText(QString("%1").arg(v));
      }
    }

    return true;
  }

  return QWidget::event(event);
}
