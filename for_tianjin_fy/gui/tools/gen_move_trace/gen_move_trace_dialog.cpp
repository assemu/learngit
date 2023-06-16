#include "gen_move_trace_dialog.h"
#include "ui_gen_move_trace_dialog.h"
#include <QtWidgets>
#include <core/util/log.h>
#include <core/util/image_util.h>
#include <gui/util/message_box_helper.h>

class TraceTableShowWidget: public QTableWidget
{
public:
  TraceTableShowWidget(const cv::Mat& img,
                       const QVector<QPoint>& top_right_path,
                       const QVector<QPoint>& bottom_right_path)
    : QTableWidget()
  {
    cv::cvtColor(img, _img, cv::COLOR_GRAY2RGB);
    _top_right_path = top_right_path;
    _bottom_right_path = bottom_right_path;

    for(auto& p : _top_right_path) {
      p.setX(p.x() - _img.cols / 2);
      p.setY(p.y() - _img.rows / 2);
    }

    for(auto& p : _bottom_right_path) {
      p.setX(p.x() - _img.cols / 2);
      p.setY(p.y() - _img.rows / 2);
    }

    setStyleSheet("QTableView::item:selected:!active{background:#287399;color:#eff0f1;}");
    setColumnCount(4);

    //设置表头内容
    QStringList header;
    header << tr("CAM3_X") << tr("CAM3_Y");
    header << tr("CAM4_X") << tr("CAM4_Y");
    setHorizontalHeaderLabels(header);

    QHeaderView* hv = horizontalHeader();
    hv->setStretchLastSection(true);
    hv->setSectionResizeMode(QHeaderView::Interactive);
    hv->setDefaultSectionSize(100);
    hv->setDefaultAlignment(Qt::AlignLeft);
    hv->setFixedHeight(25);
    hv->setSectionsClickable(true);

    for(int i = 0; i < columnCount(); i++)
      hv->resizeSection(i, 100);

    verticalHeader()->setVisible(false);
    setShowGrid(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(false);


    int rows = std::max<int>(_top_right_path.size(), _bottom_right_path.size());
    setRowCount(rows);

    int cc = 0;
    for(int r = 0; r < _top_right_path.size(); r++) {
      auto p = _top_right_path.at(r);
      auto it_x = new QTableWidgetItem(QString("%1").arg(p.x()));
      auto it_y = new QTableWidgetItem(QString("%1").arg(p.y()));
      setItem(r, cc, it_x);
      setItem(r, cc + 1, it_y);
    }

    cc = 2;
    for(int r = 0; r < _bottom_right_path.size(); r++) {
      auto p = _bottom_right_path.at(r);
      auto it_x = new QTableWidgetItem(QString("%1").arg(p.x()));
      auto it_y = new QTableWidgetItem(QString("%1").arg(p.y()));
      setItem(r, cc, it_x);
      setItem(r, cc + 1, it_y);
    }

  }

private:
  cv::Mat _img;
  QVector<QPoint> _top_right_path;
  QVector<QPoint> _bottom_right_path;
};

class TraceImageShowWidget: public QWidget
{
public:
  TraceImageShowWidget(const cv::Mat& img,
                       const QVector<QPoint>& top_right_path,
                       const QVector<QPoint>& bottom_right_path)
    : QWidget()
  {
    cv::cvtColor(img, _img, cv::COLOR_GRAY2RGB);
    _top_right_path = top_right_path;
    _bottom_right_path = bottom_right_path;
    resize(_img.cols, _img.rows);
  }

protected:
  void paintEvent(QPaintEvent* e) override
  {
    QPainter painter(this);

    if(!_img.empty()) {
      QImage image((const unsigned char*)(_img.data), _img.cols, _img.rows, _img.cols * 3, QImage::Format_RGB888);
      double r = (double)_img.cols / (double)_img.rows;
      QRect rect(0, 0, this->height() * r, this->height());
      painter.drawImage(rect, image);
    }

    {
      painter.setOpacity(0.8);
      painter.setPen(QPen(Qt::red, 4, Qt::SolidLine));
      painter.drawLines(_top_right_path);
    }

    {
      painter.setOpacity(0.8);
      painter.setPen(QPen(Qt::cyan, 4, Qt::SolidLine));
      painter.drawLines(_bottom_right_path);
    }
  }

private:
  cv::Mat _img;
  QVector<QPoint> _top_right_path;
  QVector<QPoint> _bottom_right_path;
};

GenMoveTraceDialog::GenMoveTraceDialog()
  : QDialog(), _ui(new Ui::GenMoveTraceDialog)
{
  _ui->setupUi(this);

  connect(_ui->btn, &QPushButton::clicked, this,
  [this]() {
    QString file_path = QFileDialog::getOpenFileName(NULL, tr("请选择玻璃灰度图"), ".", tr("灰度图(*.png *.jpg *.bmp)"));
    file_path = file_path.trimmed();
    if(file_path.isEmpty())
      return;

    log_info("读取图像文件...");
    auto img = readImgGRAY(file_path.toStdString());

    if(img.empty()) {
      MessageBoxHelper::show_critical("读取图片失败！");
      return;
    }

    bool useOrg = _ui->radioButton->isChecked();
    auto path = useOrg ? extractGlassEdgeOrg(img) : extractGlassEdge(img);
    log_info("提取边缘路径... size:{}", path.size());

    auto top_right_path = extractCamEdgePath(path, EdgeRight);
    log_info("计算{}路径... size:{}", cameraNameOf(EdgeRight), top_right_path.size());

    auto bottom_right_path = extractCamEdgePath(path, EdgeLeft);
    log_info("计算{}路径... size:{}", cameraNameOf(EdgeLeft), bottom_right_path.size());

    log_info("显示。。。");
    QDialog d;
    {
      auto _img_w = new TraceImageShowWidget(img, top_right_path, bottom_right_path);
      auto sw = new QScrollArea();
      sw->setWidget(_img_w);

      auto tw = new TraceTableShowWidget(img, top_right_path, bottom_right_path);

      auto sp = new QSplitter(Qt::Horizontal);
      sp->addWidget(sw);
      sp->addWidget(tw);
      sp->setSizes(QList<int>() << 600000 << 300000);

      auto ly = new QVBoxLayout();
      ly->setContentsMargins(0, 0, 0, 0);
      ly->addWidget(sp);
      d.setLayout(ly);
    }
    const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
    d.resize(availableGeometry.width() * 5 / 6, availableGeometry.height() * 5 / 6);
    d.exec();
  });
}

GenMoveTraceDialog::~GenMoveTraceDialog()
{
  delete _ui;
}
