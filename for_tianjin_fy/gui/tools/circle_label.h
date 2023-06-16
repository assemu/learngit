#ifndef GUI_PLC_CIRCLE_LABEL_H_
#define GUI_PLC_CIRCLE_LABEL_H_

#include <QLabel>

class CircleLabel : public QLabel
{
public:
  CircleLabel(QWidget* parent = 0);

  void setColor(const QColor& c);

protected:
  void paintEvent(QPaintEvent* e) override;

private:
  QColor _color;
};

#endif //GUI_PLC_CIRCLE_LABEL_H_
