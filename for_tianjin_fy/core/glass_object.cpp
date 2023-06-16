#include "glass_object.h"
#include <sstream>
#include <QPolygon>
#include <core/app_config.h>
#include <core/util/log.h>
#include <core/util/time_util.h>
#include <core/util/str_util.h>
#include <core/util/image_util.h>
#include <QMutexLocker>
#include <core/event_queue.h>
#include <core/glass_image_combiner.h>

//==================================================================================

GlassObject::GlassObject(const std::string& id, const GlassConfig& config)
  : _id(id),
    _config(config),
    _position_status(GlassObject::OnTheWay),
    _detect_status(GlassObject::DetectNotStarted)
{
  for(auto& p : allCameraPositions()) {
    _position_detect_complete[p] = false;
    _imgs[p] = std::vector<PartImg>();
    _imgs_count[p] = 0;
  }

  float scale_factor = AppConfig::get()->getFaceImageCombinerScaleFactor();
  _top_image_combiner = std::make_shared<GlassImageCombiner>(scale_factor);
  _back_image_combiner = std::make_shared<GlassImageCombiner>(scale_factor);
  _front_image_combiner = std::make_shared<GlassImageCombiner>(scale_factor);
}

GlassObject::~GlassObject()
{
  log_info("==> ~GlassObject called");
}

std::string GlassObject::id() const
{
  return _id;
}

GlassConfig GlassObject::config() const
{
  return _config;
}

bool GlassObject::putEdgePartImg(PartImg& img)
{
  if(positionStatus() < GlassObject::InFront) {
    log_warn("glass:{} put img before IoSignaled!", id());
    return false;
  }

  if(positionStatus() >= GlassObject::ScanEdgeFinished) {
    log_warn("glass:{} put edge img after ScanEdgeFinished!", id());
    return false;
  }

  if(isEdgePosition(img.position)) {
    _imgs_lock.lockForWrite();
    img.part_num = _imgs[img.position].size() + 1; //序号从1开始
    _imgs[img.position].push_back(img);
    _imgs_count[img.position]++;
    _imgs_lock.unlock();

    EventQueue::get()->pushEvent(std::make_shared<GlassPartImageEvent>(shared_from_this(), img));
    log_debug("glass:{} <== {}:{} [{}x{}]",
              _id,
              cameraNameOf(img.position),
              img.part_num,
              img.img.cols,
              img.img.rows);

    if(positionStatus() == GlassObject::InFront)
      _changePositionStatusTo(GlassObject::ScanEdgeStarted);

    if(_isEdgeImgsEnough()) {
      _changePositionStatusTo(GlassObject::ScanEdgeFinished);
      log_info("glass:{} edge scan finished!", id());
    }

    return true;
  }

  return false;
}

bool GlassObject::putFacePartImg(PartImg& img)
{
  if(positionStatus() < GlassObject::InFront) {
    log_warn("glass:{} put img before IoSignaled!", id());
    return false;
  }

  if(positionStatus() >= GlassObject::ScanFaceFinished) {
    log_warn("glass:{} put face img after ScanFaceFinished!", id());
    return false;
  }

  if(isFacePosition(img.position)) {
    _imgs_lock.lockForWrite();
    img.part_num = _imgs[img.position].size() + 1; //序号从1开始
    _imgs[img.position].push_back(img);
    _imgs_count[img.position]++;
    _imgs_lock.unlock();

    EventQueue::get()->pushEvent(std::make_shared<GlassPartImageEvent>(shared_from_this(), img));
    log_debug("glass:{} <== {}:{} [{}x{}]",
              _id,
              cameraNameOf(img.position),
              img.part_num,
              img.img.cols,
              img.img.rows);

    //图片拼合
    switch(img.position) {
      case FrontLeft:
        _front_image_combiner->addLeft(img);
        break;
      case FrontRight:
        _front_image_combiner->addRight(img);
        break;
      case BackLeft:
        _back_image_combiner->addLeft(img);
        break;
      case BackRight:
        _back_image_combiner->addRight(img);
        break;
      case TopLeft:
        _top_image_combiner->addLeft(img);
        break;
      case TopRight:
        _top_image_combiner->addRight(img);
        break;
    }

    if(positionStatus() == GlassObject::ScanEdgeFinished)
      _changePositionStatusTo(GlassObject::ScanFaceStarted);

    if(_isFaceImgsEnough()) {
      _changePositionStatusTo(GlassObject::ScanFaceFinished);
      log_info("glass:{} face scan finished!", id());
    }

    return true;
  }

  return false;
}

std::vector<PartImg> GlassObject::getPartImgs(CameraPosition position)
{
  std::vector<PartImg> re;
  _imgs_lock.lockForRead();
  if(_imgs.count(position) > 0)
    re = _imgs.at(position);
  _imgs_lock.unlock();
  return re;
}

int GlassObject::getPartImgsCount(CameraPosition position)
{
  int re = 0;
  _imgs_lock.lockForRead();
  if(_imgs_count.count(position) > 0)
    re = _imgs_count.at(position);
  _imgs_lock.unlock();
  return re;
}

QList<Flaw> GlassObject::allFlaws()
{
  QMutexLocker locker(&_flaws_mutex);
  return _flaws;
}

void GlassObject::markDetect(const PartImg& img, std::vector<Flaw>& flaws)
{
  log_debug("glass:{} mark detect position:{}, part_num:{}, ==================> _flaws:{} <===",
            id(),
            cameraNameOf(img.position),
            img.part_num,
            _flaws.size());

  //修改
  {
    QMutexLocker locker(&_flaws_mutex);
    for(auto& f : flaws) {
      std::string camera_name = cameraNameOf(f.part_img.position);
      std::string l = isEdgePosition(f.part_img.position) ? "side" : "front";

      std::stringstream ss;
      ss << camera_name + "_" + l + "_" + f.label + "_";
      ss << _flaws.size() + 1;

      f.id = ss.str(); //本玻璃中唯一ID
      _flaws.push_back(f);
    }
    qSort(_flaws.begin(), _flaws.end(), [](const Flaw & f1, const Flaw & f2)->bool{
      int v1 = f1.level();
      int v2 = f2.level();

      if(v1 == v2)
      {
        if(f1.timestamp != f2.timestamp)
          return f1.timestamp < f2.timestamp;
        else
          return f1.area > f2.area;
      } else
      {
        return v1 > v2;
      }
    });
  }

  if(detectStatus() == GlassObject::DetectNotStarted) {
    _changeDetectStatusTo(GlassObject::Detecting);
    log_info("glass:{} detect started", id());
  }

  //激发缺陷事件
  for(auto& f : flaws)
    EventQueue::get()->pushEvent(std::make_shared<GlassFlawEvent>(shared_from_this(), f));

  if(_position_detect_complete.count(img.position) <= 0) {
    _position_detect_complete[img.position] = 1;
  } else {
    _position_detect_complete[img.position] += 1;
  }

  //TODO 手动单独的处理
  bool all_complete = true;
  std::vector<CameraPosition> ps = {FrontRight, FrontLeft, BackLeft, BackRight};
  for(auto& p : ps) {
    auto begin_step = config().camera_configs[p].analyze_begin;
    auto end_step = config().camera_configs[p].analyze_end;
    all_complete = all_complete && (_position_detect_complete[p] >= (end_step - begin_step + 1));
  }

  if(all_complete)
    forceDetectDone();
}

void GlassObject::forceDetectDone()
{
  _changeDetectStatusTo(GlassObject::DetectFinished);
  log_info("glass:{} detect done!", id());

  //清空片状图缓存
  if(!AppConfig::get()->isSaveOrignalImages()) {
    for(auto& p : allCameraPositions())
      _imgs[p] = std::vector<PartImg>();
  }
}

GlassObject::PositionStatus GlassObject::positionStatus() const
{
  return _position_status;
}

GlassObject::DetectStatus GlassObject::detectStatus() const
{
  return _detect_status;
}

void GlassObject::signalIO()
{
  _timestamp = get_timestamp();
  _changePositionStatusTo(GlassObject::InFront);
  log_info("glass:{} signal io!", id());
}

void GlassObject::_changePositionStatusTo(GlassObject::PositionStatus position_status)
{
  QMutexLocker locker(&_position_status_mutex);
  auto last_status = _position_status;
  _position_status = position_status;
  log_debug("glass:{} change position status to {}", id(), _position_status);
  EventQueue::get()->pushEvent(std::make_shared<GlassPositionStatusChangedEvent>(shared_from_this(), last_status, _position_status));
}

void GlassObject::_changeDetectStatusTo(GlassObject::DetectStatus detect_status)
{
  QMutexLocker locker(&_detect_status_mutex);
  auto last_status = _detect_status;
  _detect_status = detect_status;
  log_debug("glass:{} change detect status to {}", id(), _detect_status);
  EventQueue::get()->pushEvent(std::make_shared<GlassDetectStatusChangedEvent>(shared_from_this(), last_status, _detect_status));
}

bool GlassObject::_isEdgeImgsEnough()
{
  bool re = true;
  for(auto& kv : _imgs) {
    auto position = kv.first;
    if(isEdgePosition(position)) {
      if(_imgs[position].size() < _config.camera_configs[position].image_steps) {
        re = false;
        break;
      }
    }
  }
  return re;
}

bool GlassObject::_isFaceImgsEnough()
{
  bool re = true;
  for(auto& kv : _imgs) {
    auto position = kv.first;
    //if(isFacePosition(position)) {
    if(isFacePosition(position) && position != TopLeft && position != TopRight) {
      if(_imgs[position].size() < _config.camera_configs[position].image_steps) {
        re = false;
        break;
      }
    }
  }
  return re;
}

bool GlassObject::isDetectDone()
{
  return _detect_status >= GlassObject::DetectFinished;
}

bool GlassObject::isOk()
{
  //TODO 增加新的标签的判断
  int white_count = 0;
  int black_count = 0;
  int bright_count = 0;
  int dark_count = 0;
  int dirty_count = 0;
  int scratch_count = 0;

  for(auto& f : allFlaws()) {
    if(f.label == "white") white_count++;
    if(f.label == "black") black_count++;
    if(f.label == "bright") bright_count++;
    if(f.label == "dark") dark_count++;
    if(f.label == "dirty") dirty_count++;
    if(f.label == "scratch") scratch_count++;
  }

  bool ok = true;
  if(white_count > 0 || black_count > 0 || bright_count > 0 || dark_count > 0/* || dirty_count > 0 || scratch_count > 0*/)
    ok = false;

  return ok;
}

cv::Mat GlassObject::getFrontImg(int& left_margin, int& right_margin)
{
  return _front_image_combiner->getCombinedImg(left_margin, right_margin);
}

cv::Mat GlassObject::getBackImg(int& left_margin, int& right_margin)
{
  return _back_image_combiner->getCombinedImg(left_margin, right_margin);
}

cv::Mat GlassObject::getTopImg(int& left_margin, int& right_margin)
{
  return _top_image_combiner->getCombinedImg(left_margin, right_margin);
}
