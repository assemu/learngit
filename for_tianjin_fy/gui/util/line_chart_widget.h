#ifndef GUI_VISUAL_LINE_CHART_WIDGET_H_
#define GUI_VISUAL_LINE_CHART_WIDGET_H_

#include <QMainWindow>
#include <QVector>
#include <QHash>
#include <QString>
#include <core/util/curve2d.h>

class QCustomPlot;

class LineChartWidget : public QMainWindow
{
public:
  //构造和析构函数
  LineChartWidget(QWidget* parent = 0);

  void addCurve(const Curve2d& curve, const QColor& color = Qt::gray, int line_width = 1);
  void clearAll();

private:
  QCustomPlot* plot;
};

#endif //GUI_VISUAL_LINE_CHART_WIDGET_H_
