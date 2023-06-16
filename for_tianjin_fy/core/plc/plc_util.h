#ifndef V2_PLC_UTIL_H__
#define V2_PLC_UTIL_H__

#include "plc_client.h"
#include "plc_config.h"

//下发运动轨迹配置
bool sendPlcConfigTrack(const PlcConfigTrack& c);

//下发配方配置
bool sendPlcConfigRecipe(const PlcConfigRecipe& c);

//电机控制
class MotorController
{
public:
  MotorController(const std::string& monitor_key,
                  const std::string& v_key,
                  const std::string& up_key,
                  const std::string& down_key);

  //开始停止控制
  void startControl();
  void stopControl();

  //控制移动到指定位置
  void rightMoveTo(const float pos);
  void lefttMoveTo(const float pos);

  //判断是否正在控制
  bool inControl();
  void _rightStop();
  double get_current() const {return _current_pos;};

private:
  bool _upStart();
  bool _downStart();

  std::string _monitor_key;
  std::string _v_key;
  std::string _up_key;
  std::string _down_key;

  double _current_pos;
  double _v; //速度
  bool _in_control;
};

#endif //V2_PLC_UTIL_H__
