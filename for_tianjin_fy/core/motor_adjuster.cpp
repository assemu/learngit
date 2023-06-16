#include "motor_adjuster.h"
#include <sstream>
#include <fstream>
#include <core/common.h>
#include <core/app_config.h>
#include <core/util/log.h>
#include <core/util/time_util.h>
#include <core/util/filesystem_util.h>
#include <core/scheduler.h>
#include <core/plc/plc_util.h>

MotorAdjuster::MotorAdjuster()
  : QThread(),
    _need_stop(false),
    _last_left_margin(0),
    _last_right_margin(0),
    _first_glass(true),
    _in_running(false)
{
  //左侧电机控制
  _left_controller = std::make_shared<MotorController>(
                       "GVL_Axis.edge_device_three_physical_location",
                       "GVL_Axis.McAbsolute_edge_device_three_velocity",
                       "GVL_Axis.McAbsolute_edge_device_three_up",
                       "GVL_Axis.McAbsolute_edge_device_three_down");

  //右侧电机控制
  _right_controller = std::make_shared<MotorController>(
                        "GVL_Axis.edge_device_one_physical_location",
                        "GVL_Axis.McAbsolute_edge_device_one_velocity",
                        "GVL_Axis.McAbsolute_edge_device_one_up",
                        "GVL_Axis.McAbsolute_edge_device_one_down");
}

bool MotorAdjuster::startWork()
{
  if(!_in_running) {
    start();
    _left_controller->startControl();
    _right_controller->startControl();
    log_info("MotorAdjuster started!");
    return true;
  }

  return false;
}

void MotorAdjuster::stopWork()
{
  if(_in_running) {
    log_info("try to stop MotorAdjuster ... ");
    _need_stop = true;
    _left_controller->stopControl();
    _right_controller->stopControl();
    _events.push_back(NULL); //可能阻塞，唤醒
    wait();
    log_info("MotorAdjuster stopped!");
  }
}

void MotorAdjuster::run()
{
  _need_stop = false;
  _in_running = true;
  while(!_need_stop) {
    {
      auto e = _events.wait_and_pop_front();
      process(e);
    }

    {
      std::shared_ptr<GlassEvent> e;
      while(_events.try_pop_front(e)) {
        if(!e)
          break;
        process(e);
        if(_need_stop)
          break;
      }
    }
  }

  _in_running = false;
}

void MotorAdjuster::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!event || !_in_running)
    return;

  //处理面扫结束事件
  if(event->type() == GlassEvent::GlassPositionStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassPositionStatusChangedEvent>(event);
    if(e && event->glass()->positionStatus() == GlassObject::ScanFaceFinished) {
      _events.push_back(event);
    }
  }
}

float MotorAdjuster::leftMarginToMotorPos(int left_margin)
{
  //需要考虑缩放系数 //TODO @王岩
  //公式需要重写
  //界面调用这个函数来显示数值
  float s = AppConfig::get()->getFaceImageCombinerScaleFactor(); //缩放系数
  return -0.2102 * left_margin + 508.95;
}

float MotorAdjuster::rightMarginToMotorPos(int right_margin)
{
  //需要考虑缩放系数 //TODO @王岩
  //公式需要重写
  //界面调用这个函数来显示数值
  float s = AppConfig::get()->getFaceImageCombinerScaleFactor(); //缩放系数
  return -0.2086 * right_margin + 563.45;
}

//判断是否需要控制
bool MotorAdjuster::_shouldControl(int left_margin, int right_margin)
{
  //如果正在控制 return false
  if(_left_controller->inControl() || _right_controller->inControl())
    return false;

  if(_first_glass)
    return true;

  //根据上次计算的左右边界，判断是否需要控制
  else
  {
    float last_left_motor_pos  = leftMarginToMotorPos(_last_left_margin);
    float left_motor_pos       = leftMarginToMotorPos(left_margin);
    float last_right_motor_pos = rightMarginToMotorPos(_last_right_margin);
    float right_motor_pos      =  rightMarginToMotorPos(right_margin);

    //电机位置差值达到多少，重新控制 
    //这部分集中考虑一下，如何判定一块玻璃是新玻璃，而不是临时出错出现的意外，或者算法处理异常；
    if(fabs(left_margin - _last_left_margin) <= 15 || fabs(right_margin - _last_right_margin) <= 15)
      return false;
    if(fabs(_last_left_margin + _last_right_margin - left_margin - right_margin) > 30)
    {
      return true;
    }
    // if(fabs(left_motor_pos - last_left_motor_pos) > 30
    //    || fabs(right_motor_pos - last_right_motor_pos) > 30) {
    //   return true;
    //}
  }

  //其它情况
  return false;
}

void MotorAdjuster::process(std::shared_ptr<GlassEvent> event)
{ 
  if(!event)
    return;
  //处理
  if(true) {
    auto e = std::dynamic_pointer_cast<GlassPositionStatusChangedEvent>(event);
    if(e && event->glass()->positionStatus() == GlassObject::ScanFaceFinished) {
      try {
        int left_margin = 0;
        int right_margin = 0;
        event->glass()->getFrontImg(left_margin, right_margin);
        
        if(_shouldControl(left_margin, right_margin)) 
        {
          float left_motor_pos = leftMarginToMotorPos(left_margin) * -1;
          float right_motor_pos = rightMarginToMotorPos(right_margin) * -1;
          
          //左右控制到指定位置
          _left_controller->lefttMoveTo(left_motor_pos);
          _right_controller->rightMoveTo(right_motor_pos);
          //_right_controller->_stop();

          _last_left_margin = left_margin;
          _last_right_margin = right_margin;
          _first_glass = false;
        }

      } catch(...) {
        log_error("MotorAdjuster process event failed!");
      }
    }
  }

}
