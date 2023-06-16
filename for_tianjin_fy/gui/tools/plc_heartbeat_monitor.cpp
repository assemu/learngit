#include "plc_heartbeat_monitor.h"
#include <core/util/log.h>
#include <QtWidgets>
#include <gui/util/message_box_helper.h>

const QEvent::Type PlcHeartBeatEventType = (QEvent::Type)5200;

class _PlcHeartBeatEvent : public QEvent
{
public:
  _PlcHeartBeatEvent()
    : QEvent(PlcHeartBeatEventType)
  {
  }

  ~_PlcHeartBeatEvent()
  {
  }
};

PLCHeartBeatMonitor::PLCHeartBeatMonitor(QWidget* parent)
  : QWidget(parent)
{
  _heartbeat_timer = new QTimer(this);
  _heartbeat_timer ->stop();
  connect(_heartbeat_timer, &QTimer::timeout, this,
  [this]() {
    _heartbeat_timer->stop();
    MessageBoxHelper::show_info("设备与软件心跳通信异常！");
  });
}

void PLCHeartBeatMonitor::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!event)
    return;
  if(event->type() == GlassEvent::PlcHeartBeatEvent) {
    QCoreApplication::postEvent(this, new _PlcHeartBeatEvent());
  }
}

bool PLCHeartBeatMonitor::event(QEvent* event)
{
  auto s = size();
  auto w = s.width();
  auto h = s.height();

  if(event->type() == QEvent::Wheel)
    return false;

  if(event->type() == PlcHeartBeatEventType) {
#ifndef MOCK_RUN
    _heartbeat_timer->start(20000); //重置超时时间
#endif
    return true;
  }

  return QWidget::event(event);
}

void PLCHeartBeatMonitor::startMonitorHeatbeat()
{
#ifndef MOCK_RUN
  _heartbeat_timer->start(20000); //20s
#endif
}

void PLCHeartBeatMonitor::stopMonitorHeatbeat()
{
#ifndef MOCK_RUN
  _heartbeat_timer->stop();
#endif
}
