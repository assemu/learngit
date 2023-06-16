#include "plc_config.h"
#include <core/util/json_util.h>

PlcConfigTrack::PlcConfigTrack()
{
  setConfigItem<bool>("GVL_Read_State.Model_Change_Table", false);
  setConfigItem<double>("GVL_Read_State.WriteParameter_edge_device_one_value", 0.0f);
  setConfigItem<double>("GVL_Read_State.WriteParameter_edge_device_three_value", 0.0f);

  setCAMTrackData("GVL_Read_State.coordinate_one", std::vector<double>());
  setConfigItem<int>("GVL_Read_State.array_number_one", 0);

  setCAMTrackData("GVL_Read_State.coordinate_three", std::vector<double>());
  setConfigItem<int>("GVL_Read_State.array_number_three", 0);
}

PlcConfigRecipe::PlcConfigRecipe()
{
  //TODO 需要确认这个参数还有没有用
  //setConfigItem<short>("GVL_digit_group.model", 1); //规格
  //setConfigItem<>("GVL_Axis.Model_button"); //model confirm

  setConfigItem<double>("GVL_AQ.fenduan_1", 0);
  setConfigItem<double>("GVL_AQ.fenduan_2", 100);
  setConfigItem<double>("GVL_AQ.fenduan_3", 200);
  setConfigItem<double>("GVL_AQ.fenduan_4", 300);
  setConfigItem<double>("GVL_AQ.fenduan_5", 400);
  setConfigItem<double>("GVL_AQ.fenduan_6", 600);
  setConfigItem<double>("GVL_AQ.fenduan_7", 800);
  setConfigItem<double>("GVL_AQ.fenduan_8", 1000);
  setConfigItem<double>("GVL_AQ.fenduan_9", 1200);
  setConfigItem<double>("GVL_AQ.fenduan_10", 1400);

  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_1", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_2", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_3", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_4", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_5", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_6", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_7", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_8", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_9", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_01_fenduan_10", 16000);

  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_1", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_2", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_3", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_4", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_5", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_6", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_7", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_8", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_9", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_02_fenduan_10", 11000);

  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_1", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_2", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_3", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_4", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_5", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_6", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_7", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_8", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_9", 16000);
  setConfigItem<int16_t>("GVL_AQ.AQ_03_fenduan_10", 16000);

  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_1", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_2", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_3", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_4", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_5", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_6", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_7", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_8", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_9", 11000);
  setConfigItem<int16_t>("GVL_AQ.AQ_04_fenduan_10", 11000);

  setConfigItem<std::string>("GVL_digit_group.Delay_profile_one", "00:00:00.121");
  setConfigItem<std::string>("GVL_digit_group.Delay_profile_two", "00:00:00.131");
  setConfigItem<std::string>("GVL_digit_group.Delay_profile_three", "00:00:00.141");
  setConfigItem<std::string>("GVL_digit_group.Delay_profile_four", "00:00:00.151");
  setConfigItem<std::string>("GVL_digit_group.Delaytime_glass_front_change_1", "00:00:00.5"); //正面触发延时
  setConfigItem<std::string>("GVL_digit_group.Delaytime_glass_profile_change_1", "00:00:00.121"); //侧面触发延时
  setConfigItem<double>("GVL_digit_group.Limit_position_change", 2398);
}
