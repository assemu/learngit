#ifndef V2_GLASS_EVENT_LISTENER_H_
#define V2_GLASS_EVENT_LISTENER_H_

#include <string>
#include <core/glass_object.h>
#include <core/util/json_util.h>
#include <core/util/time_util.h>
#include <core/util/log.h>

class GlassEvent
{
public:
  enum Type {
    EmptyEvent = 0,
    PlcHeartBeatEvent = 1,        //硬件设备心跳事件
    RunStartEvent = 2,
    RunStopEvent = 3,
    GlassNewEvent = 4,            //新玻璃对象事件
    GlassPositionStatusChanged = 5, //玻璃位置状态变化
    GlassDetectStatusChanged = 6, //玻璃检测状态变化
    GlassPartImageEvent = 7,      //玻璃线扫图片事件
    GlassFlawEvent = 8            //缺陷事件
  };

  GlassEvent(std::shared_ptr<GlassObject> go)
    : glass_object(go)
  {
    _timestamp = get_timestamp();
  }

  //虚析构
  virtual ~GlassEvent()
  {
  }

  //事件类型
  virtual Type type() const = 0;

  //转换成json字符串
  virtual std::string toJson() const
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      re = root.dump(2);
    } catch(...) {
      log_error("dump GlassEvent to json failed!");
    }
    return re;
  }

  //玻璃对象
  inline std::shared_ptr<GlassObject> glass() const
  {
    return glass_object;
  }

protected:
  std::shared_ptr<GlassObject> glass_object;
  int64_t _timestamp;
};

//空的事件
class EmptyEvent : public GlassEvent
{
public:
  EmptyEvent()
    : GlassEvent(NULL)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::EmptyEvent;
  }

};

//硬件设备心跳事件
class PlcHeartBeatEvent : public GlassEvent
{
public:
  PlcHeartBeatEvent()
    : GlassEvent(NULL)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::PlcHeartBeatEvent;
  }
};

//新玻璃对象
class GlassNewEvent : public GlassEvent
{
public:
  GlassNewEvent(std::shared_ptr<GlassObject> glass)
    : GlassEvent(glass)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::GlassNewEvent;
  }
};

//玻璃对象Position状态变化
class GlassPositionStatusChangedEvent : public GlassEvent
{
public:
  GlassPositionStatusChangedEvent(std::shared_ptr<GlassObject> glass,
                                  GlassObject::PositionStatus last_status,
                                  GlassObject::PositionStatus new_status)
    : GlassEvent(glass), _last_status(last_status), _new_status(new_status)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::GlassPositionStatusChanged;
  }

  inline GlassObject::PositionStatus lastStatus() const
  {
    return _last_status;
  }

  inline GlassObject::PositionStatus newStatus() const
  {
    return _new_status;
  }

  virtual std::string toJson() const override
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      root["last_status"] = (int)(lastStatus());
      root["new_status"] = (int)(newStatus());
      re = root.dump(2);
    } catch(...) {
      log_error("dump GlassPositionStatusChangedEvent to json failed!");
    }
    return re;
  }

private:
  GlassObject::PositionStatus _last_status;
  GlassObject::PositionStatus _new_status;
};

//玻璃对象Detect状态变化
class GlassDetectStatusChangedEvent : public GlassEvent
{
public:
  GlassDetectStatusChangedEvent(std::shared_ptr<GlassObject> glass,
                                GlassObject::DetectStatus last_status,
                                GlassObject::DetectStatus new_status)
    : GlassEvent(glass), _last_status(last_status), _new_status(new_status)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::GlassDetectStatusChanged;
  }

  inline GlassObject::DetectStatus lastStatus() const
  {
    return _last_status;
  }

  inline GlassObject::DetectStatus newStatus() const
  {
    return _new_status;
  }

  virtual std::string toJson() const override
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      root["last_status"] = (int)(lastStatus());
      root["new_status"] = (int)(newStatus());
      re = root.dump(2);
    } catch(...) {
      log_error("dump GlassDetectStatusChangedEvent to json failed!");
    }
    return re;
  }

private:
  GlassObject::DetectStatus _last_status;
  GlassObject::DetectStatus _new_status;
};

//玻璃扫描图片事件
class GlassPartImageEvent : public GlassEvent
{
public:
  GlassPartImageEvent(std::shared_ptr<GlassObject> glass, const PartImg& part_img)
    : GlassEvent(glass), _part_img(part_img)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::GlassPartImageEvent;
  }

  inline PartImg partImg() const
  {
    return _part_img;
  }

  virtual std::string toJson() const override
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      root["glass_id"] = glass()->id();
      root["camera_position"] = cameraNameOf(_part_img.position);
      root["width"] = _part_img.img.cols;
      root["height"] = _part_img.img.rows;
      root["part_num"] = _part_img.part_num;
      re = root.dump(2);
    } catch(...) {
      log_error("dump GlassPartImageEvent to json failed!");
    }
    return re;
  }

private:
  PartImg _part_img;
};

//玻璃缺陷事件
class GlassFlawEvent : public GlassEvent
{
public:
  GlassFlawEvent(std::shared_ptr<GlassObject> glass, const Flaw& flaw)
    : GlassEvent(glass), _flaw(flaw)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::GlassFlawEvent;
  }

  inline Flaw flaw() const
  {
    return _flaw;
  }

  virtual std::string toJson() const override
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      root["glass_id"] = glass()->id();
      root["flaw_id"] = _flaw.id;
      root["part_img_num"] = _flaw.part_img.part_num;
      root["score"] = _flaw.score;
      root["label"] = _flaw.label;
      root["realArea"] = _flaw.area;
      root["realWidth"] = _flaw.realWidth;
      root["realHeight"] = _flaw.realHeight;
      {
        auto j = nlohmann::json::array();
        j.push_back(_flaw.rect.x);
        j.push_back(_flaw.rect.y);
        j.push_back(_flaw.rect.width);
        j.push_back(_flaw.rect.height);
        root["rect"] = j;
      }

      re = root.dump(2);
    } catch(...) {
      log_error("dump GlassFlawEvent to json failed!");
    }
    return re;
  }

private:
  Flaw _flaw;
};

class RunStartEvent : public GlassEvent
{
public:
  RunStartEvent(const std::string& run_id)
    : GlassEvent(NULL), _run_id(run_id)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::RunStartEvent;
  }

  inline std::string runId() const
  {
    return _run_id;
  }

  virtual std::string toJson() const override
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      root["run_id"] = runId();
      re = root.dump(2);
    } catch(...) {
      log_error("dump RunStartEvent to json failed!");
    }
    return re;
  }

private:
  std::string _run_id;
};

class RunStopEvent : public GlassEvent
{
public:
  RunStopEvent(const std::string& run_id)
    : GlassEvent(NULL), _run_id(run_id)
  {
  }

  virtual GlassEvent::Type type() const override
  {
    return GlassEvent::RunStopEvent;
  }

  inline std::string runId() const
  {
    return _run_id;
  }

  virtual std::string toJson() const override
  {
    std::string re = "";
    try {
      auto root = nlohmann::json();
      root["event_type"] = (int)(type());
      root["timestamp"] = _timestamp;
      root["run_id"] = runId();
      re = root.dump(2);
    } catch(...) {
      log_error("dump RunStopEvent to json failed!");
    }
    return re;
  }

private:
  std::string _run_id;
};

//玻璃事件监听器
class GlassEventListener
{
public:
  //玻璃事件
  virtual void glassEvent(std::shared_ptr<GlassEvent> event) = 0;
};

#endif //V2_GLASS_EVENT_LISTENER_H_
