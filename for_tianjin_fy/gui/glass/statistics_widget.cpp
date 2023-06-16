#include "statistics_widget.h"
#include <QtWidgets>
#include <sstream>
#include <core/util/log.h>
#include <core/util/str_util.h>
#include <core/db/db.h>
#include <core/scheduler.h>

const QEvent::Type StatisticsShowEventType = (QEvent::Type)5010;

class _StatisticsShowEvent : public QEvent
{
public:
  _StatisticsShowEvent(std::shared_ptr<GlassObject> glass)
    : QEvent(StatisticsShowEventType), _glass(glass)
  {
  }

  std::shared_ptr<GlassObject> glass()
  {
    return _glass;
  }

private:
  std::shared_ptr<GlassObject> _glass;
};

StatisticsWidget::StatisticsWidget()
  : QLabel()
{
  QFont font("宋体", 16, QFont::Normal, false);
  setFont(font);

  all_detected_count = 0; //总检片数
  batch_id = "-";
  batch_detected_count = 0;
  batch_ng_count = 0;
  batch_ok_percent = 0.0f;

  AllStatisticsRecord all;
  if(DB::get()->queryAllStatisticsRecord(all))
    all_detected_count = all.detect_count;

  refresh();
}

StatisticsWidget::~StatisticsWidget()
{
}

bool StatisticsWidget::event(QEvent* event)
{
  if(event->type() == StatisticsShowEventType) {
    auto e = dynamic_cast<_StatisticsShowEvent*>(event);
    if(e) {
      auto glass = e->glass();

      {
        AllStatisticsRecord all;
        if(DB::get()->queryAllStatisticsRecord(all))
          all_detected_count = all.detect_count;
      }

      {
        BatchStatisticsRecord re;
        if(DB::get()->queryBatchStatisticsRecord(batch_id, re)) {
          batch_detected_count = re.detect_count;
          batch_ng_count = re.ng_count;
          if(batch_detected_count > 0)
            batch_ok_percent = 1.0 - (double)batch_ng_count / batch_detected_count;
        }
      }

      batch_id = Scheduler::get()->currentBatch().id;
      refresh();
    }

    return true;
  }
  return QWidget::event(event);
}

void StatisticsWidget::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!event)
    return;

  //处理一次，预显示一些数据
  {
    static bool flag = false;
    if(!flag) {
      QCoreApplication::postEvent(this, new _StatisticsShowEvent(event->glass()));
      flag = true;
    }
  }

  //处理分析结束事件
  if(event->type() == GlassEvent::GlassDetectStatusChanged) {
    auto e = std::dynamic_pointer_cast<GlassDetectStatusChangedEvent>(event);
    //if(e->newStatus() == GlassObject::DetectFinished) {
    QCoreApplication::postEvent(this, new _StatisticsShowEvent(e->glass()));
    //}
  }
}

void StatisticsWidget::refresh()
{
  QString txt = QString("总检片数:%1  当前批次:%2  批次总检: %3片  <span style='color:red;'>批次缺陷片数:%4</span>  批次合格率:%5%")
                .arg(all_detected_count)
                .arg(QString::fromStdString(batch_id))
                .arg(batch_detected_count)
                .arg(batch_ng_count)
                .arg(batch_ok_percent * 100.0, 0, 'f', 2);
  this->setText(txt);
  update();
}
