#include "circle_label.h"
#include <QPainter>
#include <QPaintEvent>

CircleLabel::CircleLabel(QWidget* parent)
  : QLabel(parent)
{
  _color = Qt::gray;
}

void CircleLabel::setColor(const QColor& c)
{
  _color = c;
  update();
}

void CircleLabel::paintEvent(QPaintEvent* e)
{
  QPainter painter(this);
  painter.setBrush(QBrush(_color));
  QRect rect(3, 3, this->width() - 6, this->height() - 6);
  painter.drawEllipse(rect);
}
