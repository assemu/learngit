#include "cvmat_show_widget.h"
#include <QtWidgets>
#include <QPaintEvent>

cvMatShowWidget::cvMatShowWidget()
  : QWidget()
{
}

void cvMatShowWidget::paintEvent(QPaintEvent* e)
{
  QPainter painter(this);
  if(!_img.empty()) {
    QImage image((const unsigned char*)(_img.data), _img.cols, _img.rows, _img.cols * 3, QImage::Format_RGB888);
    double r = (double)_img.rows / (double)_img.cols;
    QRect rect(0, 0, this->height() * r, this->height());
    painter.drawImage(rect, image);
  }
}

void cvMatShowWidget::showImgGRAY(const cv::Mat& img)
{
  cv::cvtColor(img, _img, cv::COLOR_GRAY2RGB);
  update();
}

void cvMatShowWidget::showImgBGR(const cv::Mat& img)
{
  cv::cvtColor(img, _img, cv::COLOR_BGR2RGB);
  update();
}

void cvMatShowWidget::showImgRGB(const cv::Mat& img)
{
  _img = img;
  update();
}
