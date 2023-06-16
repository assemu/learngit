#ifndef V2_SCHEDULER_H_
#define V2_SCHEDULER_H_

#include <atomic>
#include <memory>
#include <string>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QDateTime>
#include <core/common.h>
#include <core/glass_object.h>
#include <core/glass_event.h>
#include <core/part_img_consumer.h>
#include <core/io_card/io_card_signal_listener.h>
#include <core/util/thread_safe_queue.h>

class PartImgSource;
class FlawAnalyzer;
class GlassImageSaver;
class GlassDBSaver;
class MotorAdjuster;

//核心调度器，事件处理，处理PLC信号，分配扫描图片，管理玻璃对象
class Scheduler final : public PartImgConsumer, public IOCardSignalListener, public QThread
{
public:
  static Scheduler* get();

public:
  //开始运行,输入批次
  bool startWork(const Batch& batch);

  //停止运行
  void stopWork();

  //判断是否正在运行
  inline bool inRunning() const
  {
    return _in_running;
  }

public:
  //当前运行ID
  inline std::string runId() const
  {
    return _run_id;
  };

  //当前批号
  inline Batch currentBatch() const
  {
    return _batch;
  }

  //当前玻璃型号配置
  inline GlassConfig glassConfig() const
  {
    return _batch.glass_config;
  }

  std::shared_ptr<GlassObject> lastGlass(); //最后一个玻璃对象
  QList<std::shared_ptr<GlassObject>> latestGlasses(); //最近的玻璃对象

public:
  //注册玻璃事件监听器
  void registerGlassEventListener(GlassEventListener* listener);

  //删除玻璃事件监听器
  void unregisterGlassEventListener(GlassEventListener* listener);

public:
  //@see PartImgConsumer
  virtual void acceptEdgePartImg(const PartImg& img) override;
  virtual void acceptFacePartImg(const PartImg& img) override;

  //@see IOCardSignalListener
  virtual void glassComeSignal() override;
  virtual void heartBeatSignal() override;

protected:
  //@see QThread
  virtual void run() override;

private:
  Scheduler();

  void clearLatestGlasses();

  std::atomic_bool _need_stop;
  std::atomic_bool _in_running;

  QList<std::shared_ptr<GlassObject>> _glasses;
  QMutex _glasses_mutex;
  int _glass_index;

  QList<GlassEventListener*> _glass_event_listeners;
  QMutex _event_mutex;

  FlawAnalyzer* _flaw_analyzer;
  GlassImageSaver* _img_saver;
  GlassDBSaver* _db_saver;
  MotorAdjuster* _motor_adjuster;

  QDateTime _started_time;
  std::string _run_id;
  Batch _batch;
  PartImgSource* _img_source;
};

#endif //V2_SCHEDULER_H_
