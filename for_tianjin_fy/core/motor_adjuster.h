#ifndef JIANFENG_CORE_MOTOR_ADJUSTER_H_
#define JIANFENG_CORE_MOTOR_ADJUSTER_H_

#include <atomic>
#include <string>
#include <vector>
#include <QThread>
#include <memory>
#include <core/common.h>
#include <core/util/thread_safe_queue.h>
#include <core/glass_event.h>
#include <map>
#include <opencv2/opencv.hpp>

class MotorController;

class MotorAdjuster : public QThread, public GlassEventListener
{
public:
  //左右边界像素值转换到电机位置
  static float leftMarginToMotorPos(int left_margin);
  static float rightMarginToMotorPos(int right_margin);

  MotorAdjuster();

  //开始运行
  bool startWork();

  //停止运行
  void stopWork();

  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

protected:
  void run() override;

private:
  std::atomic_bool _need_stop;
  std::atomic_bool _in_running;
  ThreadSafeQueue<std::shared_ptr<GlassEvent>> _events;
  void process(std::shared_ptr<GlassEvent> e);

  int _last_left_margin;
  int _last_right_margin;
  bool _first_glass;
  bool _shouldControl(int left_margin, int right_margin);

  std::shared_ptr<MotorController> _left_controller;
  std::shared_ptr<MotorController> _right_controller;
};

#endif //JIANFENG_CORE_MOTOR_ADJUSTER_H_
