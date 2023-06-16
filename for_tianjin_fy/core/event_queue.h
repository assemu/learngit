#ifndef _J_EVENT_QUEUE_H_
#define _J_EVENT_QUEUE_H_

#include <string>
#include <memory>
#include <core/util/log.h>
#include <core/glass_event.h>
#include <core/util/thread_safe_queue.h>

class EventQueue
{
public:
  static EventQueue* get();

  //压入事件
  void pushEvent(std::shared_ptr<GlassEvent> e)
  {
    if(e) {
      log_trace("put glass:{} event:{}", e->glass() ? e->glass()->id() : "NULL", e->type());
      _events.push_back(e);
    }
  }

  //弹出事件
  bool tryPopEvent(std::shared_ptr<GlassEvent>& e)
  {
    return _events.try_pop_front(e);
  }

  std::shared_ptr<GlassEvent> waitAndPopEvent()
  {
    return _events.wait_and_pop_front();
  }

private:
  ThreadSafeQueue<std::shared_ptr<GlassEvent>> _events;
};

#endif //_J_EVENT_QUEUE_H_
