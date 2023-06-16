#include "glass_image_saver.h"
#include <sstream>
#include <fstream>
#include <core/common.h>
#include <core/app_config.h>
#include <core/util/log.h>
#include <core/util/time_util.h>
#include <core/util/filesystem_util.h>
#include <core/scheduler.h>
#include <core/analyzer/flaw_util.h>

GlassImageSaver::GlassImageSaver()
  : QThread(),
    _need_stop(false),
    _in_running(false)
{
}

bool GlassImageSaver::startWork()
{
  if(!_in_running) {
    start();
    log_info("GlassImageSaver started!");
    return true;
  }

  return false;
}

void GlassImageSaver::stopWork()
{
  if(_in_running) {
    log_info("try to stop GlassImageSaver ... ");
    _need_stop = true;
    _events.push_back(NULL);
    wait();
    log_info("GlassImageSaver stopped!");
  }
}

void GlassImageSaver::run()
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

void GlassImageSaver::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!event || !_in_running)
    return;

  //处理启动运行事件
  if(event->type() == GlassEvent::RunStartEvent) {
    auto e = std::dynamic_pointer_cast<RunStartEvent>(event);
    auto run_id = e->runId();
    auto batch_id = Scheduler::get()->currentBatch().id;
    auto batch_dir = AppConfig::get()->getDataSaveDir() + "/" + batch_id;
    auto run_dir = batch_dir + "/" + run_id;

    if(!file_exist(batch_dir)) create_path(batch_dir);
    if(!file_exist(run_dir)) create_path(run_dir);
  }

  //处理线扫图事件
  if(AppConfig::get()->isSaveOrignalImages() && event->type() == GlassEvent::GlassPartImageEvent) {
    _events.push_back(event);
  }

  //处理缺陷事件
  if(event->type() == GlassEvent::GlassFlawEvent) {
    _events.push_back(event);
  }

  //处理分析结束事件,保存拼合图像
  /*
  if(event->type() == GlassEvent::GlassPositionStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassPositionStatusChangedEvent>(event);
    if(e->newStatus() == GlassObject::ScanFaceFinished) {
      _events.push_back(event);
    }
    if(e->newStatus() == GlassObject::ScanEdgeFinished) {
      _events.push_back(event);
    }
  }
  */

  if(event->type() == GlassEvent::GlassDetectStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
    if(e->newStatus() == GlassObject::DetectFinished) {
      _events.push_back(event);
    }
  }
}

void GlassImageSaver::_save(const SaveItem& it)
{
  if(!it.img.empty() && !it.path.empty()) {
    std::vector<uchar> data;
    cv::imencode(it.format, it.img, data);
    std::ofstream ofs(it.path.c_str(), std::ios::out | std::ios::binary); //保存
    ofs.write((char*)(&data[0]), data.size());
    ofs.close();
  } else {
    log_error("save img error, img empty! path:{}", it.path);
  }
}

std::string GlassImageSaver::_glassDir(std::shared_ptr<GlassEvent> event)
{
  auto run_id = Scheduler::get()->runId();
  auto batch_dir = AppConfig::get()->getDataSaveDir() + "/" + Scheduler::get()->currentBatch().id;
  auto run_dir = batch_dir + "/" + run_id;
  auto glass_id = event->glass()->id();
  auto glass_dir = run_dir + "/" + glass_id;
  return glass_dir;
}

void GlassImageSaver::process(std::shared_ptr<GlassEvent> event)
{
  if(!event)
    return;

  //处理启动运行事件
  if(event->type() == GlassEvent::RunStartEvent) {
    auto e = std::dynamic_pointer_cast<RunStartEvent>(event);
    auto run_id = e->runId();
    auto batch_id = Scheduler::get()->currentBatch().id;
    auto batch_dir = AppConfig::get()->getDataSaveDir() + "/" + batch_id;
    auto run_dir = batch_dir + "/" + run_id;

    if(!file_exist(batch_dir)) create_path(batch_dir);
    if(!file_exist(run_dir)) create_path(run_dir);
  }

  //处理线扫图事件
  if(AppConfig::get()->isSaveOrignalImages() && event->type() == GlassEvent::GlassPartImageEvent) {
    auto e = std::dynamic_pointer_cast<GlassPartImageEvent>(event);
    auto glass_dir = _glassDir(event);
    auto part_img = e->partImg();

    if(!file_exist(glass_dir)) create_path(glass_dir);
    auto orignals_dir = glass_dir + "/orignals";
    if(!file_exist(orignals_dir)) create_path(orignals_dir);

    std::stringstream ss;
    ss << orignals_dir << "/" << cameraNameOf(part_img.position) << "_" << part_img.part_num << ".bmp";
    auto path = ss.str();

    GlassImageSaver::SaveItem it;
    it.path = path;
    it.format = ".bmp";
    it.img = part_img.img;
    _save(it);
  }

  //处理缺陷事件
  if(event->type() == GlassEvent::GlassFlawEvent) {
    auto e = std::dynamic_pointer_cast<GlassFlawEvent>(event);
    auto flaw = e->flaw();
    auto glass_dir = _glassDir(event);

    if(!file_exist(glass_dir)) create_path(glass_dir);
    auto detected_dir = glass_dir + "/detected";
    if(!file_exist(detected_dir)) create_path(detected_dir);

    {
      GlassImageSaver::SaveItem it;
      it.path = detected_dir + "/" + flaw.id + ".png";
      it.format = ".png";
      it.img = flaw.detailed_img;
      _save(it);
    }

    {
      GlassImageSaver::SaveItem it;
      it.path = detected_dir + "/" + flaw.id + "_nomark.png";
      it.format = ".png";
      it.img = flaw.detailed_img_nomark;
      _save(it);
    }
  }


  //处理分析结束事件,保存拼合图像
  //保存速度很慢，很导致事件队列积累，图像得不到释放，资源占尽崩溃
  /*
  if(0 && event->type() == GlassEvent::GlassDetectStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
    auto glass_dir = _glassDir(event);
    if(e->newStatus() == GlassObject::DetectFinished) {
      {
        GlassImageSaver::SaveItem it;
        it.path = glass_dir + "/" + "front.png";
        it.format = ".png";
        it.img = e->glass()->getFrontImg();
        if(!it.img.empty())
          _save(it);
      }
    }
  }
  */
}
