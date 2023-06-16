#ifndef V2_GUI_CURRENT_GLASS_SHOW_WIDGET_H_
#define V2_GUI_CURRENT_GLASS_SHOW_WIDGET_H_

#include <QGraphicsView>
#include <QTimer>
#include <QList>
#include <memory>
#include <core/glass_object.h>
#include "glass_show_item.h"

class QGraphicsScene;
class GlassShowItem;

class GlassShowWidget : public QGraphicsView
{
public:
  GlassShowWidget(QWidget* parent = 0, bool flaws_on_right = false);

  inline std::shared_ptr<GlassObject> glass()
  {
    return _item->glass();
  }

  void UpdateGlass(std::shared_ptr<GlassObject> g);

private:
  QGraphicsScene* _scene;
  GlassShowItem* _item;
  bool _flaws_on_right;
};

#endif //V2_GUI_CURRENT_GLASS_SHOW_WIDGET_H_
