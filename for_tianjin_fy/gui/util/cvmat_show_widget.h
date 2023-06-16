#ifndef GUI_CVMAT_SHOW_WIDGET_H_
#define GUI_CVMAT_SHOW_WIDGET_H_

#include <QWidget>
#include <opencv2/opencv.hpp>

class QPaintEvent;

class cvMatShowWidget : public QWidget
{
  Q_OBJECT;
public:
  cvMatShowWidget();

  void showImgBGR(const cv::Mat& img);
  void showImgGRAY(const cv::Mat& img);
  void showImgRGB(const cv::Mat& img);

protected:
  void paintEvent(QPaintEvent* e) override;

private:
  cv::Mat _img;
};

#endif //GUI_CVMAT_SHOW_WIDGET_H_
