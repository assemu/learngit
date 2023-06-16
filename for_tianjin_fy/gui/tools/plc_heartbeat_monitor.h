#ifndef V2_GUI_PLC_HEART_BEAT_MONITOR_H_
#define V2_GUI_PLC_HEART_BEAT_MONITOR_H_

#include <QWidget>
#include <QTimer>
#include <QList>
#include <memory>
#include <core/glass_object.h>
#include <core/glass_event.h>

class PLCHeartBeatMonitor : public QWidget,  public GlassEventListener
{
public:
  PLCHeartBeatMonitor(QWidget* parent = 0);

  //监控PLC心跳
  void startMonitorHeatbeat();
  void stopMonitorHeatbeat();

  //过滤事件
  //@see QWidget
  bool event(QEvent* event) override;

  //@see glass_event.h
  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

private:
  QTimer* _heartbeat_timer;
};

#endif //V2_GUI_PLC_HEART_BEAT_MONITOR_H_
