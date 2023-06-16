#ifndef GUI_STATISTICS_SHOW_WIDGET_H_
#define GUI_STATISTICS_SHOW_WIDGET_H_

#include <QLabel>
#include <core/glass_object.h>
#include <core/glass_event.h>

class StatisticsWidget : public QLabel,  public GlassEventListener
{
  Q_OBJECT;
public:
  StatisticsWidget();
  ~StatisticsWidget();

  //过滤事件
  bool event(QEvent* event) override;

  //@see glass_event.h
  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

private:
  int all_detected_count; //总检片数
  std::string batch_id;
  int batch_detected_count;
  int batch_ng_count;
  double batch_ok_percent;

  void refresh();
};

#endif //GUI_STATISTICS_SHOW_WIDGET_H_
