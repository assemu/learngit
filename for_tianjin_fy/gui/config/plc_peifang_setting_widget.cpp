#include "plc_peifang_setting_widget.h"
#include "ui_plc_pei_fang_widget.h"
#include <gui/util/message_box_helper.h>
#include <QtWidgets>
#include <map>
#include <sstream>
#include <core/plc/plc_client.h>
#include <core/plc/plc_util.h>
#include <core/app_config.h>

PLCPeiFangSettingWidget::PLCPeiFangSettingWidget()
  : QWidget(), _ui(new Ui::PLCPeiFangWidget)
{
  _ui->setupUi(this);

  _spin_boxes1["GVL_AQ.fenduan_1"] = _ui->fenduan_1_edit;
  _spin_boxes1["GVL_AQ.fenduan_2"] = _ui->fenduan_2_edit;
  _spin_boxes1["GVL_AQ.fenduan_3"] = _ui->fenduan_3_edit;
  _spin_boxes1["GVL_AQ.fenduan_4"] = _ui->fenduan_4_edit;
  _spin_boxes1["GVL_AQ.fenduan_5"] = _ui->fenduan_5_edit;
  _spin_boxes1["GVL_AQ.fenduan_6"] = _ui->fenduan_6_edit;
  _spin_boxes1["GVL_AQ.fenduan_7"] = _ui->fenduan_7_edit;
  _spin_boxes1["GVL_AQ.fenduan_8"] = _ui->fenduan_8_edit;
  _spin_boxes1["GVL_AQ.fenduan_9"] = _ui->fenduan_9_edit;
  _spin_boxes1["GVL_AQ.fenduan_10"] = _ui->fenduan_10_edit;
  _spin_boxes1["GVL_digit_group.Limit_position_change"] = _ui->xianwei_edit;

  _spin_boxes2["GVL_AQ.AQ_01_fenduan_1"] = _ui->AQ_01_fenduan_1_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_2"] = _ui->AQ_01_fenduan_2_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_3"] = _ui->AQ_01_fenduan_3_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_4"] = _ui->AQ_01_fenduan_4_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_5"] = _ui->AQ_01_fenduan_5_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_6"] = _ui->AQ_01_fenduan_6_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_7"] = _ui->AQ_01_fenduan_7_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_8"] = _ui->AQ_01_fenduan_8_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_9"] = _ui->AQ_01_fenduan_9_edit;
  _spin_boxes2["GVL_AQ.AQ_01_fenduan_10"] = _ui->AQ_01_fenduan_10_edit;

  _spin_boxes2["GVL_AQ.AQ_02_fenduan_1"] = _ui->AQ_02_fenduan_1_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_2"] = _ui->AQ_02_fenduan_2_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_3"] = _ui->AQ_02_fenduan_3_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_4"] = _ui->AQ_02_fenduan_4_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_5"] = _ui->AQ_02_fenduan_5_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_6"] = _ui->AQ_02_fenduan_6_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_7"] = _ui->AQ_02_fenduan_7_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_8"] = _ui->AQ_02_fenduan_8_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_9"] = _ui->AQ_02_fenduan_9_edit;
  _spin_boxes2["GVL_AQ.AQ_02_fenduan_10"] = _ui->AQ_02_fenduan_10_edit;

  _spin_boxes2["GVL_AQ.AQ_03_fenduan_1"] = _ui->AQ_03_fenduan_1_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_2"] = _ui->AQ_03_fenduan_2_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_3"] = _ui->AQ_03_fenduan_3_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_4"] = _ui->AQ_03_fenduan_4_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_5"] = _ui->AQ_03_fenduan_5_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_6"] = _ui->AQ_03_fenduan_6_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_7"] = _ui->AQ_03_fenduan_7_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_8"] = _ui->AQ_03_fenduan_8_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_9"] = _ui->AQ_03_fenduan_9_edit;
  _spin_boxes2["GVL_AQ.AQ_03_fenduan_10"] = _ui->AQ_03_fenduan_10_edit;

  _spin_boxes2["GVL_AQ.AQ_04_fenduan_1"] = _ui->AQ_04_fenduan_1_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_2"] = _ui->AQ_04_fenduan_2_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_3"] = _ui->AQ_04_fenduan_3_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_4"] = _ui->AQ_04_fenduan_4_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_5"] = _ui->AQ_04_fenduan_5_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_6"] = _ui->AQ_04_fenduan_6_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_7"] = _ui->AQ_04_fenduan_7_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_8"] = _ui->AQ_04_fenduan_8_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_9"] = _ui->AQ_04_fenduan_9_edit;
  _spin_boxes2["GVL_AQ.AQ_04_fenduan_10"] = _ui->AQ_04_fenduan_10_edit;
  _spin_boxes2["GVL_digit_group.Front_camera_light_time"] = _ui->delay_laser_edit;

  _line_edits["GVL_digit_group.Delay_profile_one"] = _ui->delay_profile_1_edit;
  _line_edits["GVL_digit_group.Delay_profile_two"] = _ui->delay_profile_2_edit;
  _line_edits["GVL_digit_group.Delay_profile_three"] = _ui->delay_profile_3_edit;
  _line_edits["GVL_digit_group.Delay_profile_four"] = _ui->delay_profile_4_edit;
  _line_edits["GVL_digit_group.Delaytime_glass_front_change_1"] = _ui->front_trigger_delay_edit;
  _line_edits["GVL_digit_group.Delaytime_glass_profile_change_1"] = _ui->side_trigger_delay_edit;

  connect(_ui->down_btn, &QPushButton::clicked, this,
  [this]() {
    if(!PLCClient::get()->isConnect()) {
      std::stringstream ss;
      ss << "PLC没有连接，当前连接地址 " << AppConfig::get()->getPLCID() << ":" << AppConfig::get()->getPLCPort();
      MessageBoxHelper::show_critical(QString::fromStdString(ss.str()));
    } else {
      PlcConfigRecipe c;
      getConfig(c);
      if(sendPlcConfigRecipe(c)) {
        MessageBoxHelper::show_info("写入成功！");
      } else {
        MessageBoxHelper::show_info("写入失败！");
      }
    }
  });

  resize(QSize(1039, 752));
}

void PLCPeiFangSettingWidget::setConfig(const PlcConfigRecipe& c)
{
  for(auto& kv : _spin_boxes1) {
    auto key = kv.first;
    auto edit = kv.second;
    double v = 0;
    if(c.getConfigItem<double>(key, v))
      edit->setValue(v);
    else
      log_error("getConfigItem<double>({}) failed", key);
  }

  for(auto& kv : _spin_boxes2) {
    auto key = kv.first;
    auto edit = kv.second;
    int16_t v = 0;
    if(c.getConfigItem<int16_t>(key, v))
      edit->setValue(v);
    else
      log_error("getConfigItem<int16_t>({}) failed", key);
  }

  for(auto& kv : _line_edits) {
    auto key = kv.first;
    auto edit = kv.second;
    std::string v;
    if(c.getConfigItem<std::string>(key, v))
      edit->setText(QString::fromStdString(v));
    else
      log_error("getConfigItem<std::string>({}) failed", key);
  }
}

void PLCPeiFangSettingWidget::getConfig(PlcConfigRecipe& c)
{
  for(auto& kv : _spin_boxes1) {
    std::string key = kv.first;
    auto edit = kv.second;
    c.setConfigItem<double>(key, edit->value());
  }

  for(auto& kv : _spin_boxes2) {
    std::string key = kv.first;
    auto edit = kv.second;
    c.setConfigItem<int16_t>(key, edit->value());
  }

  for(auto& kv : _line_edits) {
    std::string key = kv.first;
    auto edit = kv.second;
    c.setConfigItem<std::string>(key, edit->text().trimmed().toStdString());
  }

}
