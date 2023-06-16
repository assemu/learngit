#ifndef V2_IOCARD_SIGNAL_MANAGER_H_
#define V2_IOCARD_SIGNAL_MANAGER_H_

#include <memory>
#include <atomic>
#include <QThread>
#include <QMutex>
#include <QList>
#include <core/util/time_util.h>
#include <core/glass_event.h>

class IOCardSignalListener;
class IOCard;

class IOCardSignalManager : public QThread, public GlassEventListener
{
public:
  static IOCardSignalManager* get();
  ~IOCardSignalManager();

  //开始运行
  bool startWork();

  bool inRunning()
  {
    return _in_running;
  }

  //停止运行
  bool stopWork();

  //发送踢废信号
  void writeKickSignal();

  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

  //信号监听
  void registerSignalListener(IOCardSignalListener* listener);
  void unregisterSignalListener(IOCardSignalListener* listener);

protected:
  virtual void run();

private:
  IOCardSignalManager();

  std::shared_ptr<IOCard> _io_card;
  std::atomic_bool _need_stop;
  std::atomic_bool _in_running;

  QList<IOCardSignalListener*> _signal_listeners;
  QMutex _mutex;

  void _writeEndSignal();

  //踢废信号
  std::atomic_bool _need_write_kick_signal;
  std::atomic_bool _need_resume_kick_signal;
  CountDownTimer _kick_signal_resume_timer;
  void _writeKickSignal();
  void _resumeKickSignal();

  //玻璃信号
  CountDownTimer _glass_signal_blind_timer;
  bool _readGlassSignal();

  //心跳信号
  CountDownTimer _read_heartbeat_signal_blind_timer;
  CountDownTimer _write_heartbeat_signal_resume_timer;
  std::atomic_bool _need_resume_heatbeat_signal;
  bool _readAndReplyHeatBeatSignal();
  void _resumeHeatBeatSignal();
};

#endif //V2_IOCARD_SIGNAL_MANAGER_H_ 
