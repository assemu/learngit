#include "glass_show_widget.h"
#include "glass_show_item.h"
#include <QGraphicsScene>
#include <QtWidgets>
#include <core/scheduler.h>

GlassShowWidget::GlassShowWidget(QWidget* parent, bool flaws_on_right)
  : QGraphicsView(parent), _flaws_on_right(flaws_on_right)
{
  setRenderHint(QPainter::Antialiasing, true);
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
  setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
  //setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers))); //导致Splitter鼠标问题
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  _scene = new QGraphicsScene(this);
  _scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  setScene(_scene);

  _item = new GlassShowItem();
  _item->setShowFlawsOnRight(_flaws_on_right);
  _scene->addItem(_item);
  _item->setZValue(1);
}

void GlassShowWidget::UpdateGlass(std::shared_ptr<GlassObject> g)
{
  auto s = size();
  auto w = s.width();
  auto h = s.height();
  _scene->setSceneRect(0, 0, w, h);

  if(!g || _item->glass() != g) {
    _scene->clear();
    _item = new GlassShowItem(g);
    _scene->addItem(_item);
    _item->setShowFlawsOnRight(_flaws_on_right);
    _item->setZValue(1);
    _item->setBoundingRect(_scene->sceneRect());
  }

  fitInView(0, 0, w, h);
  update();
}
