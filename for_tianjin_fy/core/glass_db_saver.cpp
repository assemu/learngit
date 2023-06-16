#include "glass_db_saver.h"
#include <sstream>
#include <fstream>
#include <core/common.h>
#include <core/app_config.h>
#include <core/util/log.h>
#include <core/util/time_util.h>
#include <core/util/filesystem_util.h>
#include <core/scheduler.h>
#include <core/analyzer/flaw_util.h>
#include <core/db/objs.h>
#include <core/db/db.h>

GlassDBSaver::GlassDBSaver()
  : QThread(),
    _need_stop(false),
    _in_running(false)
{
}

bool GlassDBSaver::startWork()
{
  if(!_in_running) {
    start();
    log_info("GlassDBSaver started!");
    return true;
  }

  return false;
}

void GlassDBSaver::stopWork()
{
  if(_in_running) {
    log_info("try to stop GlassDBSaver ... ");
    _need_stop = true;
    _events.push_back(NULL); //可能阻塞，唤醒
    wait();
    log_info("GlassDBSaver stopped!");
  }
}

void GlassDBSaver::run()
{
  _need_stop = false;
  _in_running = true;
  while(!_need_stop) {
    {
      auto e = _events.wait_and_pop_front();
      process(e);
    }

    {
      std::shared_ptr<GlassEvent> e;
      while(_events.try_pop_front(e)) {
        if(!e)
          break;
        process(e);
        if(_need_stop)
          break;
      }
    }
  }

  _in_running = false;
}

void GlassDBSaver::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!event || !_in_running)
    return;

  //处理分析结束事件
  if(event->type() == GlassEvent::GlassDetectStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
    if(e->newStatus() == GlassObject::DetectFinished) {
      _events.push_back(event);
    }
  }

  //处理缺陷事件
  if(event->type() == GlassEvent::GlassFlawEvent)
    _events.push_back(event);
}

void GlassDBSaver::process(std::shared_ptr<GlassEvent> event)
{
  if(!event)
    return;

  //处理分析结束事件
  if(event->type() == GlassEvent::GlassDetectStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
    if(e->newStatus() == GlassObject::DetectFinished) {
      auto batch_id = Scheduler::get()->currentBatch().id;
      DB::get()->addGlassToStatistics(batch_id, e->glass());
    }
  }

  //处理缺陷事件
  //if(event->type() == GlassEvent::GlassFlawEvent) {
  if(0){
    auto e = std::dynamic_pointer_cast<GlassFlawEvent>(event);
    auto flaw = e->flaw();

    auto run_id = Scheduler::get()->runId();
    auto batch_dir = AppConfig::get()->getDataSaveDir() + "/" + Scheduler::get()->currentBatch().id;
    auto run_dir = batch_dir + "/" + run_id;
    auto glass_id = e->glass()->id();
    auto glass_dir = run_dir + "/" + glass_id;
    auto detected_dir = glass_dir + "/detected";

    auto mark_img_file_path = detected_dir + "/" + flaw.id + ".png";
    auto nomark_img_file_path = detected_dir + "/" + flaw.id + "_nomark.png";

    FlawRecord re;
    re.timestamp = flaw.timestamp;
    re.cam_position = flaw.part_img.position;
    re.glass_id = glass_id;
    re.batch_id = Scheduler::get()->currentBatch().id;
    re.label = flaw.label;
    re.score = flaw.score;
    re.area = flaw.area;
    re.realWidth = flaw.realWidth;
    re.realHeight = flaw.realHeight;
    re.detail_img_path = mark_img_file_path;
    re.detail_nomark_img_path = nomark_img_file_path;
    re.audited = false;
    re.audited_result = false;

    DB::get()->addFlaw(re);
  }
}
