#ifndef GUI_GLASS_TYPE_CONFIG_EDIT_H_
#define GUI_GLASS_TYPE_CONFIG_EDIT_H_

#include <QDialog>
#include <core/common.h>

class CameraConfigFilesWidget;
class PartNumConfigWidget;
class RoiConfigWidget;
class ThresholdsEditWidget;
class FilterOutWholeBottomWidget;
class PLCMovementSettingWidget;
class PLCPeiFangSettingWidget;

class GlassTypeConfigEditDialog : public QDialog
{
public:
  GlassTypeConfigEditDialog();

  void loadParam(const GlassConfig& config);
  void getParam(GlassConfig& config);

  void enableEditName();
  void disableEditName();

private:
  CameraConfigFilesWidget* _config_file_w;
  PartNumConfigWidget* _part_num_w;
  RoiConfigWidget* _roi_config_w;
  ThresholdsEditWidget* _thresholds_config_w;
  FilterOutWholeBottomWidget* _filter_out_whole_bottom_w;
  PLCMovementSettingWidget* _plc_movement_w;
  PLCPeiFangSettingWidget* _plc_peifang_w;
};

#endif //GUI_GLASS_TYPE_CONFIG_EDIT_H_
