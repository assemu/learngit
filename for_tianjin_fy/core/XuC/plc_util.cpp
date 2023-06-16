#include "plc_util.h"
#include <core/util/str_util.h>
#include <QDateTime>
#include <QStringList>
#include <thread>

bool sendPlcConfigTrack(const PlcConfigTrack& c)
{
  auto cc = PLCClient::get();

  {
    bool v = false;
    std::string key = "GVL_Read_State.Model_Change_Table";
    if(!c.getConfigItem<bool>(key, v)) {
      log_error("PLC get value of key: {} from config object failed", key);
      return false;
    }

    if(!cc->writeVar<bool>(key, v)) {
      log_error("PLC write key: {} value:{} failed", key, v);
      return false;
    }
  }

  std::vector<std::string> keys = {"GVL_Read_State.WriteParameter_edge_device_one_value",
                                   "GVL_Read_State.WriteParameter_edge_device_three_value"
                                  };
  for(auto& key : keys) {
    double v = 0;
    if(!c.getConfigItem<double>(key, v)) {
      log_error("PLC get value of key: {} from config object failed", key);
      return false;
    }

    if(!cc->writeVar<double>(key, v)) {
      log_error("PLC write key: {} value:{} failed", key, v);
      return false;
    }
  }

  std::map<std::string, std::string> maps;
  maps["GVL_Read_State.coordinate_one"] = "GVL_Read_State.array_number_one";
  maps["GVL_Read_State.coordinate_three"] = "GVL_Read_State.array_number_three";
  for(auto& kv : maps) {
    std::vector<double> data;
    if(c.getCAMTrackData(kv.first, data)) {
      bool re = true;
      re = re && cc->writeVar<int>(kv.second, data.size() / 2);
      re = re && cc->writeVector(kv.first, data);
      if(!re)
        log_error("PLC write array failed! key: {} failed", kv.first);
    }
  }

  return true;
}

bool sendPlcConfigRecipe(const PlcConfigRecipe& c)
{
  auto cc = PLCClient::get();

  std::vector<std::string> keys1 = {
    "GVL_AQ.fenduan_1",
    "GVL_AQ.fenduan_2",
    "GVL_AQ.fenduan_3",
    "GVL_AQ.fenduan_4",
    "GVL_AQ.fenduan_5",
    "GVL_AQ.fenduan_6",
    "GVL_AQ.fenduan_7",
    "GVL_AQ.fenduan_8",
    "GVL_AQ.fenduan_9",
    "GVL_AQ.fenduan_10",
    "GVL_digit_group.Limit_position_change"
  };

  for(auto& key : keys1) {
    double v = 0.0;
    if(!c.getConfigItem<double>(key, v)) {
      log_error("PLC get value of key: {} from config object failed", key);
      return false;
    }

    if(!cc->writeVar<double>(key, v)) {
      log_error("PLC write key: {} value:{} failed", key, v);
      return false;
    }
  }

  std::vector<std::string> keys2 = {
    "GVL_AQ.AQ_01_fenduan_1",
    "GVL_AQ.AQ_01_fenduan_2",
    "GVL_AQ.AQ_01_fenduan_3",
    "GVL_AQ.AQ_01_fenduan_4",
    "GVL_AQ.AQ_01_fenduan_5",
    "GVL_AQ.AQ_01_fenduan_6",
    "GVL_AQ.AQ_01_fenduan_7",
    "GVL_AQ.AQ_01_fenduan_8",
    "GVL_AQ.AQ_01_fenduan_9",
    "GVL_AQ.AQ_01_fenduan_10",

    "GVL_AQ.AQ_02_fenduan_1",
    "GVL_AQ.AQ_02_fenduan_2",
    "GVL_AQ.AQ_02_fenduan_3",
    "GVL_AQ.AQ_02_fenduan_4",
    "GVL_AQ.AQ_02_fenduan_5",
    "GVL_AQ.AQ_02_fenduan_6",
    "GVL_AQ.AQ_02_fenduan_7",
    "GVL_AQ.AQ_02_fenduan_8",
    "GVL_AQ.AQ_02_fenduan_9",
    "GVL_AQ.AQ_02_fenduan_10",

    "GVL_AQ.AQ_03_fenduan_1",
    "GVL_AQ.AQ_03_fenduan_2",
    "GVL_AQ.AQ_03_fenduan_3",
    "GVL_AQ.AQ_03_fenduan_4",
    "GVL_AQ.AQ_03_fenduan_5",
    "GVL_AQ.AQ_03_fenduan_6",
    "GVL_AQ.AQ_03_fenduan_7",
    "GVL_AQ.AQ_03_fenduan_8",
    "GVL_AQ.AQ_03_fenduan_9",
    "GVL_AQ.AQ_03_fenduan_10",

    "GVL_AQ.AQ_04_fenduan_1",
    "GVL_AQ.AQ_04_fenduan_2",
    "GVL_AQ.AQ_04_fenduan_3",
    "GVL_AQ.AQ_04_fenduan_4",
    "GVL_AQ.AQ_04_fenduan_5",
    "GVL_AQ.AQ_04_fenduan_6",
    "GVL_AQ.AQ_04_fenduan_7",
    "GVL_AQ.AQ_04_fenduan_8",
    "GVL_AQ.AQ_04_fenduan_9",
    "GVL_AQ.AQ_04_fenduan_10",

    "GVL_digit_group.Front_camera_light_time"
  };

  for(auto& key : keys2) {
    int16_t v = 0;
    if(!c.getConfigItem<int16_t>(key, v)) {
      log_error("PLC get value of key: {} from config object failed", key);
      return false;
    }

    if(!cc->writeVar<int16_t>(key, v)) {
      log_error("PLC write key: {} value:{} failed", key, v);
      return false;
    }
  }

  std::vector<std::string> keys3 = {
    "GVL_digit_group.Delay_profile_one",
    "GVL_digit_group.Delay_profile_two",
    "GVL_digit_group.Delay_profile_three",
    "GVL_digit_group.Delay_profile_four",
    "GVL_digit_group.Delaytime_glass_front_change_1",
    "GVL_digit_group.Delaytime_glass_profile_change_1"
  };

  for(auto& key : keys3) {
    std::string v = "00:00:00.0";
    if(!c.getConfigItem<std::string>(key, v)) {
      log_error("PLC get value of key: {} from config object failed", key);
      return false;
    }

    QTime t0 = QTime::fromString("00:00:00.0", "hh:mm:ss.z");
    int msecs = t0.msecsTo(QTime::fromString(QString::fromStdString(v), "hh:mm:ss.z"));
    if(!cc->writeVar<unsigned long>(key, (unsigned long)msecs)) {
      log_error("PLC write key: {} value:{} failed", key, v);
      return false;
    }
  }

  return true;
}

MotorController::MotorController(const std::string& monitor_key,
                                 const std::string& v_key,
                                 const std::string& up_key,
                                 const std::string& down_key)
  : _monitor_key(monitor_key),
    _v_key(v_key),
    _up_key(up_key),
    _down_key(down_key),
    _current_pos(0.0),
    _v(0.0),
    _in_control(false)
{
}

void MotorController::startControl()
{
  auto c = PLCClient::get();
  if(!c->isConnect()) {
    log_error("MotorController plc is not connected");
    return;
  }

  c->setNotificationCallBack(_monitor_key.c_str(), 20, sizeof(double), [this](void* data) {
    _current_pos = *((double*)data);
  });

  //读取速度值
  if(!c->readVar<double>(_v_key.c_str(), _v)) {
    log_error("MotorController can not read velocity value! key:{}", _v_key);
  }
}

void MotorController::stopControl()
{
  auto c = PLCClient::get();
  if(c->isConnect())
    c->delNotificationCallBack(_monitor_key.c_str());
}

void MotorController::moveTo(float pos)
{
  if(pos > 0 || pos < -500)
	  return ;
  auto c = PLCClient::get();
  if(!c->isConnect()) {
    log_error("MotorController plc is not connected");
    return;
  }

  if(_v > 0) {
    //开启一个线程来控制
    std::thread t([this, pos]() {
      //等到取到当前值
      while(true) {
        if(fabs(_current_pos) < 0.0005) { //当前电机值还没取到,等20ms
          std::this_thread::sleep_for(std::chrono::milliseconds(20));
          continue;
        } else {
          break;
        }
      }

      //目标值和当前值很相近，不用控制
      if(fabs(pos - _current_pos) < 3)
        return;

      //@王岩，此处需要确认
      if(pos - _current_pos > 0) { //向下
        if(this->_downStart()) { //启动向下
          //监控当前值
          while(true) {
            if(pos - _current_pos < 3) { //到达位置，或者可能超过了
              this->_stop();
              break;
            }
          }
        }
      } else { //向上
        if(this->_upStart()) { //启动向上
          //监控当前值
          while(true) {
            if(pos - _current_pos > -3) { //到达位置，或者可能超过了
              this->_stop();
              break;
            }
          }
        }
      }

    });
    t.detach(); //不管这线程了，控制完就自动结束了
  }
}

//向上运动
bool MotorController::_upStart()
{
  auto c = PLCClient::get();
  if(c->writeVar<bool>(_up_key, true)) {
    _in_control = true;
    return true;
  }

  log_error("MotorController start up failed!");
  return false;
}

//向下运动
bool MotorController::_downStart()
{
  auto c = PLCClient::get();
  if(c->writeVar<bool>(_down_key, true)) {
    _in_control = true;
    return true;
  }

  log_error("MotorController start down failed!");
  return false;
}

//停止运动
void MotorController::_stop()
{
  auto c = PLCClient::get();
  c->writeVar<bool>(_up_key, false);
  c->writeVar<bool>(_down_key, false);
  _in_control = false;
}

bool MotorController::inControl()
{
  return _in_control;
}
