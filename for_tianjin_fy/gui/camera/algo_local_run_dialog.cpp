#include "algo_local_run_dialog.h"
#include "ui_algo_local_run_dialog.h"
#include <QtWidgets>
#include <core/util/log.h>
#include <core/util/image_util.h>
#include <core/util/time_util.h>
#include <gui/util/message_box_helper.h>
#include <gui/util/cvmat_show_widget.h>
#include <core/analyzer/detectors/detectors_pool.h>
#include <core/analyzer/flaw_util.h>

AlgoLocalRunDialog::AlgoLocalRunDialog()
  : QDialog(), _ui(new Ui::AlgoLocalRunDialog)
{
  setWindowIcon(QIcon(":logo"));
  setWindowTitle("算法本地验证");

  QWidget* pw = new QWidget();
  pw->setFixedHeight(100);
  _ui->setupUi(pw);

  _cw = new cvMatShowWidget();

  auto ly = new QVBoxLayout();
  ly->setContentsMargins(0, 0, 0, 0);
  ly->addWidget(pw);
  ly->addWidget(_cw);
  setLayout(ly);

  resize(800, 900);

  connect(_ui->chooseFileBtn, &QPushButton::clicked, this,
  [this]() {
    QString file_path = QFileDialog::getOpenFileName(NULL, tr("请选择相机灰度图"), ".", tr("灰度图(*.png *.jpg *.jpeg *.bmp)"));
    file_path = file_path.trimmed();
    if(file_path.isEmpty())
      return;

    _ui->lineEdit->setText(file_path);
  });

  connect(_ui->okBtn, &QPushButton::clicked, this,
  [this]() {
    auto file_path = _ui->lineEdit->text().trimmed();
    if(file_path.isEmpty()) {
      MessageBoxHelper::show_critical("请选择图片路径！");
      return;
    }

    auto img = readImgGRAY(file_path.toStdString());
    if(img.empty()) {
      MessageBoxHelper::show_critical("读取图片失败！");
      return;
    }

    cv::cvtColor(img, img, cv::COLOR_GRAY2RGB);
    std::vector<BoxInfo> boxes;
    bool is_edge = _ui->edgeRadio->isChecked();
    TimerClock t;
    if(is_edge) {
      DetectorsPool::get()->detectEdgeImg(img, boxes);
      log_info("detect edge img {}x{} cost: {} ms", img.cols, img.rows, t.getTimerMilliSec());
    } else {
      DetectorsPool::get()->detectFaceImg(img, boxes);
      log_info("detect face img {}x{} cost: {} ms", img.cols, img.rows, t.getTimerMilliSec());
    }

    cv::cvtColor(img, img, cv::COLOR_RGB2BGR);
    drawMarkers(img, boxes);

    _cw->showImgBGR(img);
  });
}

AlgoLocalRunDialog::~AlgoLocalRunDialog()
{
}
