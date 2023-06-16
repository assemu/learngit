#include "glass_type_config_edit_dialog.h"
#include "thresholds_edit_widget.h"
#include "camera_config_files_widget.h"
#include "part_num_config_widget.h"
#include "roi_config_widget.h"
#include "filter_out_whole_bottom_widget.h"
#include "plc_peifang_setting_widget.h"
#include "plc_movement_setting_widget.h"
#include <gui/util/message_box_helper.h>
#include <QtWidgets>
#include <QStringList>

GlassTypeConfigEditDialog::GlassTypeConfigEditDialog()
  : QDialog()
{
  auto tab = new QTabWidget();

  _config_file_w = new CameraConfigFilesWidget();
  _part_num_w = new PartNumConfigWidget();
  _roi_config_w = new RoiConfigWidget();
  _thresholds_config_w = new ThresholdsEditWidget();
  _filter_out_whole_bottom_w = new FilterOutWholeBottomWidget();
  _plc_movement_w = new PLCMovementSettingWidget();
  _plc_peifang_w = new PLCPeiFangSettingWidget();

  tab->addTab(_config_file_w, "型号名称&相机配置文件");
  tab->addTab(_part_num_w, "配置图像序号");
  tab->addTab(_roi_config_w, "配置边缘ROI");
  tab->addTab(_thresholds_config_w, "配置缺陷过滤");
  tab->addTab(_filter_out_whole_bottom_w, "配置底边水渍过滤");
  tab->addTab(_plc_movement_w, "配置PLC运动轨迹");
  tab->addTab(_plc_peifang_w, "配置PLC配方");

  auto okBtn = new QPushButton("确定");
  auto cancelBtn = new QPushButton("取消");
  okBtn->resize(100, 50);
  cancelBtn->resize(100, 50);
  okBtn->setIcon(QIcon(":icons/OK.png"));

  auto hly = new QHBoxLayout();
  hly->setContentsMargins(5, 5, 5, 5);
  hly->addSpacing(50);
  hly->addWidget(okBtn);
  hly->addSpacing(100);
  hly->addWidget(cancelBtn);
  hly->addStretch();

  connect(okBtn, &QPushButton::clicked, this,
  [this]() {
    //检查参数
    GlassConfig c;
    getParam(c);

    if(c.name.empty()) {
      MessageBoxHelper::show_critical("型号名称不能为空！");
      return;
    }

    {
      std::vector<std::string> cams;
      for(auto& kv : c.camera_configs) {
        auto position = kv.first;
        auto config = kv.second;
        if(config.config_file.empty()) {
          cams.push_back(cameraNameOf(position));
        }
      }
      if(cams.size() > 0) {
        QStringList ss;
        for(auto& cam : cams)
          ss << QString::fromStdString(cam);
        MessageBoxHelper::show_critical(QString("以下相机工位配置文件不能为空！\n[%1] ").arg(ss.join(",")));
        return;
      }
    }

    this->done(1);
  });

  connect(cancelBtn, &QPushButton::clicked, this,
  [this]() {
    this->done(0);
  });

  auto ly = new QVBoxLayout();
  ly->setContentsMargins(0, 0, 0, 0);
  ly->addWidget(tab);
  ly->addLayout(hly);
  setLayout(ly);
  resize(1100, 900);
}

void GlassTypeConfigEditDialog::loadParam(const GlassConfig& config)
{
  std::map<CameraPosition, std::string> _config_files_map;
  std::map<CameraPosition, int> _begin_steps_map;
  std::map<CameraPosition, int> _end_steps_map;
  std::map<CameraPosition, int> _steps_map;
  std::map<CameraPosition, cv::Rect> _rois_map;
  for(auto& kv : config.camera_configs) {
    auto position = kv.first;
    auto config = kv.second;
    _config_files_map[position] = config.config_file;
    _begin_steps_map[position] = config.analyze_begin;
    _end_steps_map[position] = config.analyze_end;
    _steps_map[position] = config.image_steps;
    _rois_map[position] = config.edge_rois;
  }

  _config_file_w->setName(config.name);
  _config_file_w->setCameraConfigFiles(_config_files_map);

  _part_num_w->setBeginSteps(_begin_steps_map);
  _part_num_w->setEndSteps(_end_steps_map);
  _part_num_w->setSteps(_steps_map);
  _roi_config_w->setRoiConfig(_rois_map);

  _thresholds_config_w->loadParam(config.camera_configs);
  _filter_out_whole_bottom_w->loadParam(config);

  _plc_movement_w->setConfig(config.track_config);
  _plc_peifang_w->setConfig(config.recipe_config);
}

void GlassTypeConfigEditDialog::getParam(GlassConfig& config)
{
  config.name = _config_file_w->getName();
  for(auto& kv : _config_file_w->getCameraConfigFiles())
    config.camera_configs[kv.first].config_file = kv.second;

  for(auto& kv : _part_num_w->getBeginSteps())
    config.camera_configs[kv.first].analyze_begin = kv.second;
  for(auto& kv : _part_num_w->getEndSteps())
    config.camera_configs[kv.first].analyze_end = kv.second;
  for(auto& kv : _part_num_w->getSteps())
    config.camera_configs[kv.first].image_steps = kv.second;

  for(auto& kv : _roi_config_w->getRoiConfig())
    config.camera_configs[kv.first].edge_rois = kv.second;

  _thresholds_config_w->getParam(config.camera_configs);
  _filter_out_whole_bottom_w->getParam(config);

  _plc_movement_w->getConfig(config.track_config);
  _plc_peifang_w->getConfig(config.recipe_config);
}

void GlassTypeConfigEditDialog::enableEditName()
{
  _config_file_w->enableEditName();
}

void GlassTypeConfigEditDialog::disableEditName()
{
  _config_file_w->disableEditName();
}
