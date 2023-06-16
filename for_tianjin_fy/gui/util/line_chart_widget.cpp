#include "line_chart_widget.h"
#include <QtWidgets>
#include <random>
#include <QCoreApplication>
#include <gui/util/qcustomplot.h>

LineChartWidget::LineChartWidget(QWidget* parent)
  : QMainWindow(parent)
{
  plot = new QCustomPlot(this);
  plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
  plot->resize(300, 300);

  // 设置坐标轴名称
  plot->xAxis->setLabel(tr("X"));
  plot->yAxis->setLabel(tr("Y"));

  // 设置背景色
  plot->setBackground(QColor(220, 220, 220));

  // 设置x/y轴文本色、轴线色、字体等
  {
    plot->xAxis->setTickLabelColor(Qt::black);
    plot->xAxis->setLabelColor(Qt::black);
    plot->xAxis->setBasePen(QPen(Qt::black));
    plot->xAxis->setTickPen(QPen(Qt::black));
    plot->xAxis->setSubTickPen(QPen(Qt::black));
    plot->xAxis->grid()->setPen(QPen(QColor(150, 150, 150)));
    QFont xFont = plot->xAxis->labelFont();
    xFont.setPixelSize(20);
    plot->xAxis->setLabelFont(xFont);
  }

  {
    plot->yAxis->setTickLabelColor(Qt::black);
    plot->yAxis->setLabelColor(Qt::black);
    plot->yAxis->setBasePen(QPen(Qt::black));
    plot->yAxis->setTickPen(QPen(Qt::black));
    plot->yAxis->setSubTickPen(QPen(Qt::black));
    plot->yAxis->grid()->setPen(QPen(QColor(150, 150, 150)));
    QFont yFont = plot->yAxis->labelFont();
    yFont.setPixelSize(20);
    plot->yAxis->setLabelFont(yFont);
  }

  setContextMenuPolicy(Qt::NoContextMenu);
  setCentralWidget(plot);
}

void LineChartWidget::addCurve(const Curve2d& curve, const QColor& color, int line_width)
{
  QCPGraph* pGraph = plot->addGraph();
  pGraph->setPen(QPen(color, line_width));

  double max_x = 0, min_x = 0, min_y = 0, max_y = 0;
  for(int i = 0; i < curve.size(); i++) {
    double x = curve.x(i);
    double y = curve.y(i);
    pGraph->addData(x, y);

    min_x = x < min_x ? x : min_x;
    min_y = y < min_y ? y : min_y;
    max_x = x > max_x ? x : max_x;
    max_y = y > max_y ? y : max_y;
  }

  plot->rescaleAxes(false); //包含不可见曲线
  plot->yAxis->setRange(min_y - fabs(max_y - min_y) * 0.1, max_y + fabs(max_y - min_y) * 0.1);
  plot->xAxis->setRange(min_x - fabs(max_x - min_x) * 0.1, max_x + fabs(max_x - min_x) * 0.1);
  plot->replot(QCustomPlot::rpQueuedReplot);
}

void LineChartWidget::clearAll()
{
  plot->clearGraphs();
  plot->replot(QCustomPlot::rpQueuedReplot);
}
