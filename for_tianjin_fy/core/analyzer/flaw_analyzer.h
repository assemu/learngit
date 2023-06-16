#ifndef V2_FLAW_ANALYZER_H_
#define V2_FLAW_ANALYZER_H_

#include <atomic>
#include <memory>
#include <string>
#include <QList>
#include <core/glass_event.h>
#include <core/util/thread_safe_queue.h>

class DetectorsPool;
class FlawAnalyzeThread;

class FlawAnalyzer : public GlassEventListener
{
public:
  FlawAnalyzer();
  ~FlawAnalyzer();

  //开始工作
  bool startWork();

  //停止工作
  void stopWork();

  //@see GlassEventListener
  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

private:
  ThreadSafeQueue<std::shared_ptr<GlassPartImageEvent>> _part_img_events;

  void filterFlaws(std::vector<Flaw>& flaws);
  QList<FlawAnalyzeThread*> _work_threads;

  std::atomic_bool _in_running;

  friend class FlawAnalyzeThread;
};

#endif //V2_FLAW_ANALYZER_H_
