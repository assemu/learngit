#ifndef GUI_ICON_UTIL_H_
#define GUI_ICON_UTIL_H_

#include <QIcon>
#include <QPixmap>

static QIcon makeIconByColor(const QColor& c)
{
  QPixmap p(32, 32);
  p.fill(c);
  return QIcon(p);
}

#endif //GUI_ICON_UTIL_H_
