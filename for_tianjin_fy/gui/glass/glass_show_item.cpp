#include "glass_show_item.h"
#include <QPainter>
#include <QDateTime>
#include <QGraphicsScene>
#include <core/util/image_util.h>
#include <core/util/time_util.h>
#include <core/util/log.h>

GlassShowItem::GlassShowItem()
  : QGraphicsItem(), _glass(nullptr), _show_flaws_on_right(false)
{
  setFlag(QGraphicsItem::ItemIsSelectable, false);
  setFlag(QGraphicsItem::ItemIsFocusable, false);
}

GlassShowItem::GlassShowItem(std::shared_ptr<GlassObject> glass)
  : QGraphicsItem(), _glass(glass), _show_flaws_on_right(false)
{
  setFlag(QGraphicsItem::ItemIsSelectable, false);
  setFlag(QGraphicsItem::ItemIsFocusable, false);
}

static QPen getFlawDrawPen(const Flaw& f)
{
  QPen pen(Qt::green, 2);
  if(f.level() == 2)
    pen = QPen(Qt::red, 5);
  else if(f.level() == 1)
    pen = QPen(Qt::yellow, 2);
  else
    pen = QPen(Qt::green, 2);

  return pen;
}

void GlassShowItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget)
{
  Q_UNUSED(widget);
  auto rect = boundingRect();
  painter->save(); //画灰色背景
  {
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::gray);
    painter->drawRect(rect);
  }
  painter->restore();

  if(!_glass)
    return;

  cv::Mat front_img;
  int left_margin = 0;
  int right_margin = 0;
  cv::rotate(_glass->getFrontImg(left_margin, right_margin),
             front_img, cv::ROTATE_90_CLOCKWISE); //顺时针旋转90度

  double fr = (double)front_img.rows / (double)front_img.cols; // H/W
  double rr = (double)(rect.height()) / (double)(rect.width());// H/W

  //默认底部留1/4用于显示缺陷图
  auto front_rect = rect.adjusted(0, 0, 0, -rect.height() / 4.0);
  auto flaw_rect = rect.adjusted(0, front_rect.height(), 0, 0);
  auto front_img_rect = front_rect.adjusted(0, 0, -(front_rect.width() - (double)(front_rect.height()) / fr), 0); //图像的实际
  if(fr / rr < 0.75) 
  { //图像宽度占满,剩余都给缺陷图
    flaw_rect = rect.adjusted(0, rect.width() * fr, 0, 0);
    front_rect = rect.adjusted(0, 0, 0, -flaw_rect.height());
    front_img_rect = front_rect;
  }
  if(_show_flaws_on_right) {
    front_rect = rect.adjusted(0, 0, -rect.width() / 4.0, 0); //默认
    flaw_rect = rect.adjusted(front_rect.width(), 0, 0, 0);
    front_img_rect = front_rect.adjusted(0, 0, 0, -(front_rect.height() - (double)(front_rect.width()) * fr)); //图像的实际
    if(rr / fr < 0.75) {
      flaw_rect = rect.adjusted(rect.height() / fr, 0, 0, 0);
      front_rect = rect.adjusted(0, 0, -flaw_rect.width(), 0);
      front_img_rect = front_rect;
    }
  }

  painter->save(); //画灰色背景
  {
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::lightGray);
    painter->drawRect(flaw_rect);
  }
  painter->restore();

  if(!front_img.empty()) {
    painter->save();
    {
      int W = front_rect.width();
      int H = front_rect.height();

      if(W <= 0 || H <= 0)
        return;

      cv::Mat cv_img;
      cv::Mat resized_img;
      cv::resize(front_img, resized_img, cv::Size(front_img_rect.width(), front_img_rect.height()), 0, 0, cv::INTER_LINEAR);
      levelAdjust(resized_img, cv_img, 5); //调整色阶
      QImage image((const unsigned char*)(cv_img.data), cv_img.cols, cv_img.rows, cv_img.cols * 3, QImage::Format_RGB888);
      painter->drawImage(front_img_rect, image);

      painter->save();
      painter->translate(front_rect.topLeft());
      auto _glass_id = _glass->id(); //画glass id
      QFont font("宋体", 14, QFont::Bold, false);
      painter->setFont(font);
      painter->setPen(Qt::black);
      painter->drawText(QRect(W / 20, H / 20, W - W / 20, H / 5), QString::fromStdString(_glass_id));
      painter->restore();

      //绘制正面和反面缺陷
      drawFaceFlaws(painter, front_img_rect);

      //画相机片状图数量
      drawPartImagesCount(painter, front_img_rect);

      //画左右margin
      drawMarginNumber(painter, front_img_rect, left_margin, right_margin);

      //绘制边缘路径
      if(_glass->positionStatus() >= GlassObject::ScanFaceFinished) {
        double hr = (double)resized_img.rows / front_img.rows;
        double wr = (double)resized_img.cols / front_img.cols;
        _scaler = wr;
        if(_edge_path.size() <= 0) {
          //图片太小会导致边缘提取出错误的边缘
          //使用较大图片提取边缘，再缩放边缘

          auto path = extractGlassEdgeF(front_img, 0, 2, _rect_output);
          //auto path = extractGlassEdge(front_img);
          _edge_path.resize(path.size());
          for(int i = 0; i < path.size(); i++) {
            auto p = path[i];
            _edge_path[i] = QPoint(p.x() * wr, p.y() * hr);
          }
        }
        try {
          drawCamEdgePathFlawFred(painter, Qt::cyan, EdgeLeft, front_img_rect, wr, hr, _rect_output); //右上
        } catch(...) {}
        /*
        try{
        	drawCamEdgePathAndFlawOf(painter, Qt::cyan, EdgeRight, front_rect); //右上
        }
        catch (...) {}
        try {
        	drawCamEdgePathAndFlawOf(painter, Qt::magenta, EdgeLeft, front_rect); //右下
        }catch(...){}
        */
      }
    }
    painter->restore();

    //结果
    painter->save();
    drawFinalResult(painter, front_img_rect);
    painter->restore();

    //瑕疵图片列表
    painter->save();
    drawFlawImages(painter, _glass->allFlaws(), flaw_rect);
    painter->restore();
  }

  //画框
  painter->save();
  {
    painter->setOpacity(0.4);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(rect);
    painter->setOpacity(1.0);
  }
  painter->restore();
}

static QVector<QPoint> getPathOfPercent(const QVector<QPoint>& path, double L, float percent)
{
  if(path.size() <= 1)
    return path;

  QVector<QPoint> re;
  re.append(path[0]);
  double l = 0;
  for(int i = 1; i < path.size(); i++) {
    auto p1 = path[i - 1];
    auto p2 = path[i];
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double s = sqrt(dx * dx + dy * dy);
    l += s;
    if(l <= L * percent) {
      re.append(p2);
      continue;
    }
    break;
  }
  return re;
}

double percentOf(std::shared_ptr<GlassObject> glass, CameraPosition position, int part_num)
{
  double percent = 0.0;
  int begin_step = glass->config().camera_configs[position].analyze_begin;
  int end_step = glass->config().camera_configs[position].analyze_end;
  if(part_num >= end_step)
    percent = 1.0;
  else if(part_num >= begin_step)
    percent = ((double)(part_num - begin_step + 1) / (end_step - begin_step + 1));
  //std::cout <<" Part_Num: " << part_num << std::endl;
  return percent;
}
double percentOfF(std::shared_ptr<GlassObject> glass, CameraPosition position, int part_num, int delta)
{
  double percent = 0.0;
  int begin_step = glass->config().camera_configs[TopLeft].analyze_begin;
  int end_step = glass->config().camera_configs[TopLeft].analyze_end;

  if(part_num >= end_step)
    percent = 1.0;
  else if(part_num >= begin_step)
    percent = ((double)(part_num * 640 - delta) / (end_step * 1000.0));
  //std::cout << " Part_Num: " << part_num << std::endl;
  return percent;
}

void GlassShowItem::drawCamEdgePathAndFlawOf(QPainter* painter, const QColor& color, CameraPosition position, const QRectF& grect)
{
  if(_edge_path.size() <= 0)
    return;

  auto scan_path = extractCamEdgePath(_edge_path, position);
  if(scan_path.size() <= 1)
    return;

  double L = 0.0; //周长
  for(int i = 1; i < scan_path.size(); i++) {
    auto p1 = scan_path[i - 1];
    auto p2 = scan_path[i];
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double s = sqrt(dx * dx + dy * dy);
    L += s;
  }

  //绘制路径
  if(1) {
    painter->setPen(QPen(color, 2, Qt::SolidLine));
    painter->drawLines(scan_path);
  }

  //绘制缺陷
  {
    for(auto& f : _glass->allFlaws()) {
      if(f.part_img.position == position) {
        int part_num = f.part_img.part_num;
        double percent = percentOf(_glass, position, part_num);
        //double percent = percentOfF(_glass, position, part_num, 3300); //经验数据
        auto path = getPathOfPercent(scan_path, L, 1 - percent);
        if(path.size() > 0) {
          auto p = path[path.size() - 1];

          painter->save();
          painter->translate(p);
          painter->setPen(getFlawDrawPen(f));
          int W = grect.width() / 50;
          int H = grect.height() / 50;
          //Fredrick
          cv::Rect rt = f.rect;
          //std::cout << "---->PATH: "  << " P:" << " PERCENT: " << percent << std::endl;
          QRectF box(-H, -W, W * 2, H * 2);
          painter->drawEllipse(box.center(), H, H);
          painter->restore();
        }
      }
    }
  }
}

void GlassShowItem::drawCamEdgePathFlawFred(QPainter* painter, const QColor& color, CameraPosition position, const QRectF& grect, double wr, double hr, const cv::Rect& rect_out)
{
  //绘制缺陷 _edge_path 遍历轮廓
  if(_edge_path.size() <= 0) return;
  {
    int mid_yval = 0;
    for(int m = 0; m < _edge_path.size(); m++) {
      mid_yval += _edge_path[m].y();
    }
    mid_yval = mid_yval / _edge_path.size();
    int total = _glass->config().camera_configs[FrontLeft].analyze_end;
    int edge_width = _glass->config().camera_configs[FrontLeft].edge_rois.width;
    int start_idx = _glass->config().camera_configs[position].analyze_begin;
    int end_idx = _glass->config().camera_configs[position].analyze_end;

    //std::cout << "---->" << edge_width << " " << _glass->config().camera_configs[EdgeLeft].image_steps << std::endl;
    for(auto& f : _glass->allFlaws()) {
      int part_num = f.part_img.part_num;//part_num 应该是flaw index
      double percent = percentOf(_glass, position, part_num);
      //std::cout << "Rect:" << rect_out.size() << " "<< wr << " " << hr << std::endl;

      int flaw_x = (rect_out.x + (1 - percent) * rect_out.width) * wr;
      int flaw_y = mid_yval;

      if(f.part_img.position == EdgeLeft) {
        for(int i = 0; i < _edge_path.size(); i++) {
          if(_edge_path[i].x() == flaw_x && _edge_path[i].y() <= flaw_y)
            flaw_y = _edge_path[i].y();
        }
        painter->save();
        painter->translate(flaw_x, flaw_y);
        painter->setPen(getFlawDrawPen(f));
        int W = grect.width() / 50;
        int H = grect.height() / 50;
        QRectF box(-W, -H, W * 2, H * 2);
        painter->drawEllipse(box.center(), H, H);
        painter->restore();
      } else if(f.part_img.position == EdgeRight) {
        for(int i = 0; i < _edge_path.size(); i++) {
          if(_edge_path[i].x() == flaw_x && _edge_path[i].y() >= flaw_y)
            flaw_y = _edge_path[i].y();
        }
        painter->save();
        painter->translate(flaw_x, flaw_y);
        painter->setPen(getFlawDrawPen(f));
        int W = grect.width() / 50;
        int H = grect.height() / 50;
        QRectF box(-W, -H, W * 2, H * 2);
        painter->drawEllipse(box.center(), H, H);
        painter->restore();
      }
    }
  }
}

//在矩形框rect中，排版显示缺陷细节图
void GlassShowItem::drawFlawImages(QPainter* painter, const QList<Flaw>& flaws, const QRectF& rect)
{
  int L = std::max(rect.width(), rect.height());
  int yp = 2;
  int xp = 2;

  painter->save();
  painter->translate(rect.topLeft());
  for(auto& f : _glass->allFlaws()) {
    cv::Mat fimg;
    cv::resize(f.detailed_img, fimg,
               cv::Size(f.detailed_img.cols * 1.0 / f.detailed_img.rows * (L / 8), (L / 8)),
               0, 0, cv::INTER_LINEAR);//先缩放到统一的高度

    cv::cvtColor(fimg, fimg, cv::COLOR_BGR2RGB);
    int fw = fimg.cols;
    int fh = fimg.rows;

    if(xp + fw > rect.width()) {
      xp = 0;
      yp += fh + 2;
    }

    if(yp + fh > rect.height())
      break;

    painter->save();
    painter->translate(xp, yp);
    QImage image((const unsigned char*)(fimg.data), fw, fh, fimg.cols * 3, QImage::Format_RGB888);
    painter->drawImage(QRect(0, 0, fw, fh), image);
    painter->setPen(QPen(Qt::white, 1));
    painter->drawRect(QRect(0, 0, fw, fh));
    xp += fw + 2;
    painter->restore();
  }
  painter->restore();
}

void GlassShowItem::drawFinalResult(QPainter* painter, const QRectF& front_rect)

{
  int W = front_rect.width();
  int H = front_rect.height();
  if(_glass->isDetectDone()) {
    QString txt;
    if(_glass->isOk()) {
      painter->setPen(Qt::green);
      txt = "OK";
    } else {
      painter->setPen(Qt::red);
      txt = "NG";
    }
    auto trect = QRectF(W - W * 3 / 32, 8, W / 4, H / 4);
    QFont font("宋体", 30, QFont::Bold, false);
    painter->setFont(font);
    painter->translate(front_rect.topLeft());
    painter->drawText(trect, Qt::AlignTop | Qt::AlignLeft, txt);
    painter->translate(-front_rect.topLeft());
  }
}

//将所有的面缺陷，定位并绘制
void GlassShowItem::drawFaceFlaws(QPainter* painter, const QRectF& front_rect)
{
  painter->save();
  for(auto& f : _glass->allFlaws()) {
    auto position = f.part_img.position;
    if(isFacePosition(position)) {
      double DH = f.part_img.img.rows;
      //image_steps-1以后，比例更接近
      double H = DH * (_glass->config().camera_configs[position].image_steps);
      double W = f.part_img.img.cols;
      int part_num = f.part_img.part_num;
      cv::Rect rt = f.rect;

      //计算检测框在整图中的归一化表示
      bool is_left = (position == FrontLeft || position == BackLeft || position == TopLeft);
      //Fredrick
      //double rx = (double)(rt.x + (is_left ? 0 : W)) / (W + W) ;
      //double ry = ((double)(rt.y) + (part_num - 1) * DH) / H;
      double rx = (double)(rt.x + (is_left ? 0 : W)) / (W + W);
      double ry = ((double)(rt.y) + (part_num - 1) * DH) / H;
      double rw = (double)(rt.width) / (W + W);
      double rh = (double)(rt.height) / H;

      double FW = front_rect.width();
      double FH = front_rect.height() ;
      QRectF box((1 - ry - rh) * FW, (rx) * FH, rh * FW, rw * FH); //顺时针转90度
      //////////////////////////////////////////////////////////////////////////
      //if(!flawContained(cv::Rect(_rect_output.x * _scaler,_rect_output.y*_scaler,_rect_output.width*_scaler, _rect_output.height*_scaler), (1 - ry - rh) * FW, (rx)* FH)) continue;

      painter->setPen(getFlawDrawPen(f));
      painter->save();
      painter->translate(box.center());
      //正面用正方形 //修改正方形 三角形 菱形尺寸 FH/100 -> FH/80
      if(position == FrontLeft || position == FrontRight) {
        //painter->drawRect(-FH / 50, -FH / 50, FH / 50 * 1.2, FH / 50*1.2);
        QPainterPath path;
        QPointF p1(0, -FH / 50);
        QPointF p2(-FH / 50, FH / 50);
        QPointF p3(FH / 50, FH / 50);
        path.moveTo(p1);
        path.lineTo(p2);
        path.lineTo(p3);
        path.lineTo(p1);
        painter->drawPath(path);
      } else if(position == BackLeft || position == BackRight) {
        //背面用三角形
        QPainterPath path;
        QPointF p1(0, FH / 50);
        QPointF p2(-FH / 50, -FH / 50);
        QPointF p3(FH / 50, -FH / 50);
        path.moveTo(p1);
        path.lineTo(p2);
        path.lineTo(p3);
        path.lineTo(p1);
        painter->drawPath(path);
      } else if(position == TopLeft || position == TopRight) {
        //透视面用菱形
        QPainterPath path;
        QPointF p1(0, -FH / 50);
        QPointF p2(-FH / 50, 0);
        QPointF p3(0, FH / 50);
        QPointF p4(FH / 50, 0);
        path.moveTo(p1);
        path.lineTo(p2);
        path.lineTo(p3);
        path.lineTo(p4);
        path.lineTo(p1);
        painter->drawPath(path);
      }
      painter->restore();
    }
  }

  painter->restore();
}

void GlassShowItem::drawPartImagesCount(QPainter* painter, const QRectF& front_rect)
{
  int W = front_rect.width();
  int H = front_rect.height();

  painter->save();
  painter->translate(front_rect.topLeft());
  int x = 10;
  for(auto& p : allCameraPositions()) {
    auto p_str = cameraNameOf(p);
    int count = _glass->getPartImgsCount(p);
    QString txt = QString("%1:%2").arg(QString::fromStdString(p_str)).arg(count);
    QFont font("宋体", 8, QFont::Bold, false);
    painter->setFont(font);
    painter->setPen(Qt::black);
    painter->drawText(QRect(x, front_rect.height() - 30, 80, 20), txt);
    x += 80;
  }
  QString rect_msg = QString(" Moter4:") + QString("%1").arg((int)((_rect_output.tl().y * _scaler) * 1.072 - 490))
                     + QString(" Moter3:") + QString("%1").arg((int)(((H - _rect_output.tl().y * _scaler - std::min(_rect_output.height, _rect_output.width) * _scaler) - 35) * 1.072 - 490));
  painter->drawText(QRect(x, front_rect.height() - 30, 200, 20), rect_msg);
  painter->restore();
}

void GlassShowItem::drawMarginNumber(QPainter* painter, const QRectF& front_rect, int left_margin, int right_margin)
{
  int W = front_rect.width();
  int H = front_rect.height();

  painter->save();
  painter->translate(front_rect.topLeft());
  int x = 10;
  QString margin_txt = QString(" 电机3:") + QString("%1").arg(-0.2028*right_margin + 556.03)
                       +QString("电机4:") + QString("%1").arg(-0.205*left_margin + 506.83);
  painter->drawText(QRect(x, front_rect.height() - 60, 600, 20), margin_txt);
  painter->restore();
}
