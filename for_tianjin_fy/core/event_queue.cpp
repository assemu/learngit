#include "event_queue.h"

EventQueue* EventQueue::get()
{
  static EventQueue* _inst = NULL;
  if(_inst == NULL)
    _inst = new EventQueue();
  return _inst;
}
