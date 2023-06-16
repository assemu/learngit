#ifndef V2_GUI_GLASS_SHOW_ITEM_H_
#define V2_GUI_GLASS_SHOW_ITEM_H_

#include <memory>
#include <QGraphicsItem>
#include <core/glass_object.h>
#include <QList>
#include <QPoint>
#include <core/app_config.h>
#include <opencv2/opencv.hpp>


class QPainter;

class GlassShowItem : public QGraphicsItem
{
public:
  GlassShowItem();
  GlassShowItem(std::shared_ptr<GlassObject> glass);

  QRectF boundingRect() const override
  {
    return _boundingRect;
  }

  void setBoundingRect(const QRectF& rect)
  {
    _boundingRect = rect;
  }

  inline std::shared_ptr<GlassObject> glass()
  {
    return _glass;
  }

  void setShowFlawsOnRight(bool v)
  {
    _show_flaws_on_right = v;
  }

  void paint(QPainter* painter, const QStyleOptionGraphicsItem* item, QWidget* widget) override;

private:
  std::shared_ptr<GlassObject> _glass;
  QRectF _boundingRect;
  bool _show_flaws_on_right;

  QVector<QPoint> _edge_path;
  cv::Rect _rect_output;
  double _scaler;

  void drawCamEdgePathAndFlawOf(QPainter* painter, const QColor& color, CameraPosition position, const QRectF& grect);
  void drawCamEdgePathFlawFred(QPainter* painter, const QColor& color, CameraPosition position, const QRectF& grect, double wr, double hr, const cv::Rect& rect_out);

  void drawFlawImages(QPainter* painter, const QList<Flaw>& flaws, const QRectF& rect);
  void drawFinalResult(QPainter* painter, const QRectF& front_rect);
  void drawFaceFlaws(QPainter* painter, const QRectF& front_rect);
  void drawPartImagesCount(QPainter* painter, const QRectF& front_rect);
  void drawMarginNumber(QPainter* painter, const QRectF& front_rect, int left_margin, int right_margin);
};

#endif //V2_GUI_GLASS_SHOW_ITEM_H_
