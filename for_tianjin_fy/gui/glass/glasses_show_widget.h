#ifndef V2_GUI_GLASSES_SHOW_WIDGET_H_
#define V2_GUI_GLASSES_SHOW_WIDGET_H_

#include <QWidget>
#include <QMutex>
#include <QTimer>
#include <QList>
#include <memory>
#include <core/glass_object.h>
#include <core/glass_event.h>

class GlassShowWidget;

class GlassesShowWidget : public QWidget,  public GlassEventListener
{
public:
  GlassesShowWidget(QWidget* parent = 0);

  //过滤事件
  //@see QWidget
  bool event(QEvent* event) override;

  //@see glass_event.h
  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

private:
  GlassShowWidget* _current_w;
  GlassShowWidget* _history_w1;
  GlassShowWidget* _history_w2;
};

#endif //V2_GUI_GLASSES_SHOW_WIDGET_H_
