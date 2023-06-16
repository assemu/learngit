#include "scheduler.h"
#include "glass_image_saver.h"
#include "glass_db_saver.h"
#include "motor_adjuster.h"
#include "event_queue.h"
#include <QMutexLocker>
#include <core/util/log.h>
#include <core/account/account_manager.h>
#include <core/io_card/io_card_signal_manager.h>
#include <core/analyzer/flaw_analyzer.h>
#include <core/plc/plc_util.h>

#ifdef MOCK_RUN
#include <core/mock/mocker.h>
#else
#include <core/camera/camera_part_img_source.h>
#endif

Scheduler* Scheduler::get()
{
  static Scheduler* _inst = NULL;
  if(_inst == NULL)
    _inst = new Scheduler();
  return _inst;
}

Scheduler::Scheduler()
  : _need_stop(false),
    _in_running(false),
    _img_source(NULL),
    _glass_index(0)
{
  _flaw_analyzer = new FlawAnalyzer();
  _img_saver = new GlassImageSaver();
  _db_saver = new GlassDBSaver();

#ifndef MOCK_RUN
  _motor_adjuster = new MotorAdjuster();
#endif
}

void Scheduler::registerGlassEventListener(GlassEventListener* listener)
{
  QMutexLocker locker(&_event_mutex);
  if(listener)
    _glass_event_listeners.append(listener);
}

void Scheduler::unregisterGlassEventListener(GlassEventListener* listener)
{
  QMutexLocker locker(&_event_mutex);
  _glass_event_listeners.removeAll(listener);
}

void Scheduler::acceptEdgePartImg(const PartImg& img)
{
  auto gs = latestGlasses();
  auto g = lastGlass();
  if(g) {
    //找到匹配的玻璃对象,只考虑最新的3个玻璃对象
    for(int i = 1; i < 3 && i < gs.size(); i++) {
      auto g2 = gs[i];
      if((g->timestamp() - g2->timestamp()) > 6000)  //与前一张玻璃相隔超过3秒，不考虑
        break;
      if(g2->positionStatus() >= GlassObject::ScanEdgeFinished) //g2的侧扫完了
        break;

      g = g2;
    }

    //接收图片
    if(g->positionStatus() < GlassObject::ScanEdgeFinished) {
      PartImg part_img = img;
      g->putEdgePartImg(part_img); //带上序号
    }
  }
}

void Scheduler::acceptFacePartImg(const PartImg& img)
{
  auto gs = latestGlasses();
  auto g = lastGlass();
  if(g) {
    //找到匹配的玻璃对象,只考虑最新的3个玻璃对象
    for(int i = 1; i < 3 && i < gs.size(); i++) {
      auto g2 = gs[i];
      if((g->timestamp() - g2->timestamp()) > 6000)  //与前一张玻璃相隔超过3秒，不考虑
        break;
      if(g2->positionStatus() >= GlassObject::ScanFaceFinished) //g2的面扫完了
        break;

      g = g2;
    }

    //接收图片
    if(g->positionStatus() < GlassObject::ScanFaceFinished) {
      PartImg part_img = img;
      g->putFacePartImg(part_img); //带上序号
    }
  }
}

void Scheduler::heartBeatSignal()
{
  EventQueue::get()->pushEvent(std::make_shared<PlcHeartBeatEvent>());
}

void Scheduler::glassComeSignal()
{
  log_debug("io cart signal received, there comes a new glass.");

  //1.处理之前的玻璃对象,强制结束分析
  for(int i = 0; i < _glasses.size(); i++) {
    auto g = _glasses.at(i);
    if(g->detectStatus() == GlassObject::DetectFinished)
      break;
    //if(i >= 2 && g->detectStatus() < GlassObject::DetectFinished)
    else
      g->forceDetectDone();
  }

  //2.创建一个新的玻璃对象
  _glass_index++;
  auto date_time = QDateTime::currentDateTime();
  auto _glass_id = QString("%1-%2").arg(date_time.toString("yyMMddhhmmss")).arg(_glass_index).toStdString();
  auto glass = std::make_shared<GlassObject>(_glass_id, _batch.glass_config);
  {
    QMutexLocker locker(&_glasses_mutex);
    _glasses.push_front(glass);
    while(_glasses.size() > 5) //最多保留5个
      _glasses.removeLast();
  }
  glass->signalIO();
  EventQueue::get()->pushEvent(std::make_shared<GlassNewEvent>(glass)); //新玻璃对象事件
}

bool Scheduler::startWork(const Batch& batch)
{
  //如果没登录
  if(!AccountManager::get()->currentLoginedAccount()) {
    log_error("scheduler could not start with no account logined!");
    return false;
  }

  if(!inRunning()) {
    _batch = batch; //批号


#ifndef MOCK_RUN
    if(0) {
      //下发PLC参数
      if(sendPlcConfigRecipe(_batch.glass_config.recipe_config)) {
        log_info("send plc peifang params success!");
      } else {
        log_error("send plc peifang params failed!");
        return false;
      }
      if(sendPlcConfigTrack(_batch.glass_config.track_config)) {
        log_info("send plc yundongguiji params success!");
      } else {
        log_error("send plc yundongguiji params failed!");
        return false;
      }
    }
#endif

    //3.缺陷分析器
    registerGlassEventListener(_flaw_analyzer);
    _flaw_analyzer->startWork();
    log_debug("flaw_analyzer started!");

    //2.图像源
#ifdef MOCK_RUN
    _img_source = Mocker::get();
#else
    _img_source = CameraPartImgSource::get();
#endif
    _img_source->registerPartImgConsumer(this);
    if(!_img_source->startWork(_batch.glass_config)) {
      log_error("ImgSource start failed!");
      return false;
    }
    log_debug("ImgSource started!");

    //4.统计数据保存
    registerGlassEventListener(_db_saver);
    _db_saver->startWork();
    log_debug("db_saver started!");

    //5.图像保存
    registerGlassEventListener(_img_saver);
    _img_saver->startWork();
    log_debug("img_saver started!");

    //运动轨迹调整
#ifndef MOCK_RUN
    registerGlassEventListener(_motor_adjuster);
    _motor_adjuster->startWork();
    log_debug("motor_adjuster started!");
#endif

    //6.IO板卡信号
    log_info("start IOCardSignalManager ...");
    IOCardSignalManager::get()->registerSignalListener(this);
    if(!IOCardSignalManager::get()->inRunning()) {
      if(!IOCardSignalManager::get()->startWork()) {
        log_error("IOCardSignalManager start failed!");
        return false;
      }
    }
    registerGlassEventListener(IOCardSignalManager::get());
    log_debug("iocard signal manager started!");

    //线程启动
    start();
    _started_time = QDateTime::currentDateTime();
    _run_id = _started_time.toString("RUN_yyyy_MM_dd_hh_mm_ss").toStdString();
    EventQueue::get()->pushEvent(std::make_shared<RunStartEvent>(_run_id));
    log_debug("run started!");

    return true;
  }

  return false;
}

void Scheduler::stopWork()
{
  if(inRunning()) {
    log_info("try to stop Scheduler ...");

    //1.缺陷分析器停止工作
    unregisterGlassEventListener(_flaw_analyzer);
    _flaw_analyzer->stopWork();

    //2.统计数据保存停止工作
    unregisterGlassEventListener(_db_saver);
    _db_saver->stopWork();

    //3.图片保存停止工作
    unregisterGlassEventListener(_img_saver);
    _img_saver->stopWork();

#ifndef MOCK_RUN
    unregisterGlassEventListener(_motor_adjuster);
    _motor_adjuster->stopWork();
#endif

    //4. IO信号停止工作
    unregisterGlassEventListener(IOCardSignalManager::get());
    IOCardSignalManager::get()->unregisterSignalListener(this);
    IOCardSignalManager::get()->stopWork();

    //5.图像源停止工作
    _img_source->unregisterPartImgConsumer(this);
    _img_source->stopWork();

    //6.清除玻璃对象
    clearLatestGlasses();

    _need_stop = true;
    EventQueue::get()->pushEvent(std::make_shared<EmptyEvent>());//解除阻塞
    wait();

    log_info("stop Scheduler success");
  }
}

void Scheduler::run()
{
  _need_stop = false;
  _in_running = false;
  while(!_need_stop) {
    _in_running = true;

    //事件机制
    std::shared_ptr<GlassEvent> e = EventQueue::get()->waitAndPopEvent();
    if(e && e->type() != GlassEvent::EmptyEvent) {
      log_trace("scheduler notify glass:{} event:{}", e->glass() ? e->glass()->id() : "NULL", e->type());
      QMutexLocker locker(&_event_mutex);
      for(auto& l : _glass_event_listeners)
        l->glassEvent(e);
    }
  }

  //通知停止事件
  {
    auto e = std::make_shared<RunStopEvent>(runId());
    QMutexLocker locker(&_event_mutex);
    for(auto& l : _glass_event_listeners)
      l->glassEvent(e);
  }

  //运行结束
  _in_running = false;
}

std::shared_ptr<GlassObject> Scheduler::lastGlass()
{
  QMutexLocker locker(&_glasses_mutex);
  if(_glasses.size() > 0)
    return _glasses.first();
  return NULL;
}

QList<std::shared_ptr<GlassObject>> Scheduler::latestGlasses()
{
  QMutexLocker locker(&_glasses_mutex);
  return _glasses;
}

void Scheduler::clearLatestGlasses()
{
  QMutexLocker locker(&_glasses_mutex);
  _glasses.clear();
}
