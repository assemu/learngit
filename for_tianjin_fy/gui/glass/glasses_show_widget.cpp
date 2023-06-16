.#include "glasses_show_widget.h"
#include "glass_show_widget.h"
#include "glass_show_item.h"
#include <core/util/image_util.h>
#include <core/util/log.h>
#include <QGraphicsLineItem>
#include <QtWidgets>
#include <QGraphicsScene>
#include <gui/util/message_box_helper.h>
#include <core/scheduler.h>

const QEvent::Type GlassShowUpdateEventType = (QEvent::Type)5000;
const QEvent::Type GlassScanFinishedEventType = (QEvent::Type)5002;

class _GlassScanFinishedEvent : public QEvent
{
public:
  _GlassScanFinishedEvent(std::shared_ptr<GlassObject> glass)
    : QEvent(GlassScanFinishedEventType), _glass(glass)
  {
  }

  std::shared_ptr<GlassObject> glass()
  {
    return _glass;
  }

private:
  std::shared_ptr<GlassObject> _glass;
};

class _GlassShowUpdateEvent : public QEvent
{
public:
  _GlassShowUpdateEvent(std::shared_ptr<GlassObject> glass)
    : QEvent(GlassShowUpdateEventType), _glass(glass)
  {
  }

  std::shared_ptr<GlassObject> glass()
  {
    return _glass;
  }

private:
  std::shared_ptr<GlassObject> _glass;
};

GlassesShowWidget::GlassesShowWidget(QWidget* parent)
  : QWidget(parent)
{
  _current_w = new GlassShowWidget(this);
  _history_w1 = new GlassShowWidget(this, true);
  _history_w2 = new GlassShowWidget(this, true);

  auto vsp = new QSplitter(Qt::Vertical);
  vsp->addWidget(_history_w1);
  vsp->addWidget(_history_w2);
  vsp->setSizes(QList<int>() << 800000 << 400000);

  auto sp = new QSplitter(Qt::Horizontal);
  sp->addWidget(_current_w);
  sp->addWidget(vsp);
  sp->setSizes(QList<int>() << 400000 << 400000);

  auto ly = new QVBoxLayout();
  ly->setContentsMargins(0, 0, 0, 0);
  ly->addWidget(sp);
  setLayout(ly);
}

void GlassesShowWidget::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!event)
    return;

  try {
    if(event->type() == GlassEvent::GlassDetectStatusChanged) {
      auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
      if(e && event->glass()->detectStatus() == GlassObject::DetectFinished) {
        QCoreApplication::postEvent(this, new _GlassShowUpdateEvent(event->glass()));
      }
    }

    if(event->type() == GlassEvent::GlassPositionStatusChanged) {
      auto e = std::dynamic_pointer_cast<GlassPositionStatusChangedEvent>(event);
      if(e && event->glass()->positionStatus() == GlassObject::ScanFaceFinished) {
        QCoreApplication::postEvent(this, new _GlassScanFinishedEvent(event->glass()));
      }
    }

    //面扫之后才接受缺陷事件
    if(event->type() == GlassEvent::GlassFlawEvent) {
      auto e = std::dynamic_pointer_cast<GlassFlawEvent>(event);
      if(e && event->glass()->positionStatus() >= GlassObject::ScanFaceFinished) {
        QCoreApplication::postEvent(this, new _GlassShowUpdateEvent(event->glass()));
      }
    }
  } catch(...)
  {}
}

void _get_next_two_glasses(std::shared_ptr<GlassObject> g,
                           std::shared_ptr<GlassObject>& g1,
                           std::shared_ptr<GlassObject>& g2)
{
  auto list = Scheduler::get()->latestGlasses();
  int N = list.size();
  for(int i = 0; i < N; i++) {
    if(list.at(i) == g) {
      if(i + 1 < N)
        g1 = list.at(i + 1);
      if(i + 2 < N)
        g2 = list.at(i + 2);
    }
  }
}

bool GlassesShowWidget::event(QEvent* event)
{
  auto s = size();
  auto w = s.width();
  auto h = s.height();

  if(event->type() == QEvent::Wheel)
    return false;

  try {
    if(event->type() == GlassScanFinishedEventType) {
      auto e = dynamic_cast<_GlassScanFinishedEvent*>(event);
      if(e) {
        auto g = e->glass();
        _current_w->UpdateGlass(g);
        std::shared_ptr<GlassObject> g1;
        std::shared_ptr<GlassObject> g2;
        _get_next_two_glasses(g, g1, g2);
        _history_w1->UpdateGlass(g1);
        _history_w2->UpdateGlass(g2);
      }

      return true;
    }

    if(event->type() == GlassShowUpdateEventType) {
      auto e = dynamic_cast<_GlassShowUpdateEvent*>(event);
      if(e) {
        _current_w->UpdateGlass(_current_w->glass());
        _history_w1->UpdateGlass(_history_w1->glass());
        _history_w2->UpdateGlass(_history_w2->glass());
      }

      return true;
    }
  } catch(...) {
  }
  return QWidget::event(event);
}
