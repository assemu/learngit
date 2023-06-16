#ifndef GUI_FILTER_OUT_WHOLE_BOTTOM_WIDGET_H
#define GUI_FILTER_OUT_WHOLE_BOTTOM_WIDGET_H

#include <QWidget>
#include <core/common.h>

namespace Ui
{
class FilterOutWholeBottomWidget;
}

class FilterOutWholeBottomWidget : public QWidget
{
public:
  FilterOutWholeBottomWidget();

  void loadParam(const GlassConfig& c);
  void getParam(GlassConfig& c);

private:
  Ui::FilterOutWholeBottomWidget* _ui;
};

#endif //GUI_FILTER_OUT_WHOLE_BOTTOM_WIDGET_H
