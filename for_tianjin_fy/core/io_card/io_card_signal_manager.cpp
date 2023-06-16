#include "io_card_signal_manager.h"
#include "io_card_signal_listener.h"
#include "AdvIOCard.h"
#include <thread>
#include <core/util/log.h>
#include <core/util/time_util.h>
#include <core/mock/mocker.h>

IOCardSignalManager* IOCardSignalManager::get()
{
  static IOCardSignalManager* _inst = NULL;
  if(_inst == NULL)
    _inst = new IOCardSignalManager();
  return _inst;
}

IOCardSignalManager::IOCardSignalManager()
  : QThread(),
    _need_stop(false),
    _in_running(false),
    _need_write_kick_signal(false),
    _need_resume_kick_signal(false),
    _need_resume_heatbeat_signal(false)
{
  setTerminationEnabled(false);

#ifdef MOCK_RUN
  _io_card = Mocker::get()->getIOCard();
#else
  _io_card = std::make_shared<AdvIOCard>();
#endif
}

IOCardSignalManager::~IOCardSignalManager()
{
  stopWork();
}

bool IOCardSignalManager::startWork()
{
  if(!_in_running) {
    this->start(QThread::HighPriority);
    return true;
  }

  return false;
}

void IOCardSignalManager::run()
{
  bool Status = false;

  bool blind = false;
  auto last_t = std::chrono::system_clock::now();

  _need_stop = false;
  _in_running = true;
  while(!_need_stop) {
    //踢废信号
    _writeKickSignal();
    _resumeKickSignal();

    //玻璃信号
    if(_readGlassSignal()) {
      _mutex.lock();
      for(auto& p : _signal_listeners)
        p->glassComeSignal(); //报告玻璃信号
      _mutex.unlock();
    }

    //心跳信号
    if(_readAndReplyHeatBeatSignal()) {
      _mutex.lock();
      for(auto& p : _signal_listeners)
        p->heartBeatSignal(); //报告玻璃信号
      _mutex.unlock();
    }
    _resumeHeatBeatSignal();

    QThread::yieldCurrentThread(); //出让CPU,有可能错误信号
    //thread_spin_ms(5); //或者自旋一会,注意不要错过信号
  }

  _in_running = false;

  //停止线程后，发送给IO主板一个信号
  _writeEndSignal();
}

void IOCardSignalManager::_resumeHeatBeatSignal()
{
  if(_need_resume_heatbeat_signal && !_write_heartbeat_signal_resume_timer.hasTimeLeft()) {
    _need_resume_heatbeat_signal = false;
    bool re =  _io_card->writeBit(0, 1, 0);
    log_info("heartbeat signal resumed: {}", re);
  }
}

bool IOCardSignalManager::_readAndReplyHeatBeatSignal()
{
  static WORD last_value = 0;
  bool re = false;
  if(!_read_heartbeat_signal_blind_timer.hasTimeLeft()) {
    WORD value;
    if(_io_card->readBit(0, 2, value)) {
      if(value == 1 && last_value == 0) {
        log_debug("got heartbeat signal");
        //信号持续小于200ms
        _read_heartbeat_signal_blind_timer.reset_ms(200); //blind 200ms

        //回复
        bool wre = _io_card->writeBit(0, 1, 1);
        _write_heartbeat_signal_resume_timer.reset_ms(100);
        _need_resume_heatbeat_signal = true;
        log_info("reply heatbeat signal: {}", wre);

        re = true;
      }
      last_value = value;
    }
  }

  return re;
}

bool IOCardSignalManager::_readGlassSignal()
{
  bool re = false;
  if(!_glass_signal_blind_timer.hasTimeLeft()) {
    WORD value;
    if(_io_card->readBit(0, 0, value) && value == 1) {
      log_debug("got glass signal");
      _glass_signal_blind_timer.reset_ms(200); //blind 200ms
      re = true;
    }
  }
  return re;
}

void IOCardSignalManager::_writeKickSignal()
{
  if(_need_write_kick_signal) {
    _need_write_kick_signal = false;
    bool re = _io_card->writeBit(0, 0, 1);
    _kick_signal_resume_timer.reset_ms(30); //30ms
    _need_resume_kick_signal = true;
    log_info("writed kick signal: {}", re);
  }
}

void IOCardSignalManager::_resumeKickSignal()
{
  if(_need_resume_kick_signal && !_kick_signal_resume_timer.hasTimeLeft()) {
    _need_resume_kick_signal = false;
    bool re = _io_card->writeBit(0, 0, 0);
    log_info("kick signal resumed: {}", re);
  }
}

void IOCardSignalManager::_writeEndSignal()
{
  //参考GlassVision_Vidi_Pytorch项目源码
  log_debug("write end signal to the io_card");
  for(int i = 0; i < 4; i++)
    _io_card->writeBit(0, i, 0x00);
}

bool IOCardSignalManager::stopWork()
{
  if(_in_running) {
    log_info("try to stop IOCardSignalManager ... ");
    _need_stop = true;
    wait();
    log_info("IOCardSignalManager stopped!");
    return true;
  }
  return false;
}

void IOCardSignalManager::registerSignalListener(IOCardSignalListener* listener)
{
  _mutex.lock();
  _signal_listeners.append(listener);
  _mutex.unlock();
}

void IOCardSignalManager::unregisterSignalListener(IOCardSignalListener* listener)
{
  _mutex.lock();
  _signal_listeners.removeAll(listener);
  _mutex.unlock();
}

void IOCardSignalManager::writeKickSignal()
{
  _need_write_kick_signal = true;
}

void IOCardSignalManager::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(event->type() == GlassEvent::GlassDetectStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
    if(e->newStatus() == GlassObject::DetectFinished) {
      if(!e->glass()->isOk()) {
        writeKickSignal();
      }
    }
  }
}
