#ifndef JIANFENG_CORE_GLASS_DB_SAVER_H_
#define JIANFENG_CORE_GLASS_DB_SAVER_H_

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

class GlassDBSaver : public QThread, public GlassEventListener
{
public:
  GlassDBSaver();

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
};

#endif //JIANFENG_CORE_GLASS_DB_SAVER_H_
