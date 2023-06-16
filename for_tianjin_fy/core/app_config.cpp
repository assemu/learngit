#include "app_config.h"
#include <toml.hpp>
#include <iostream>
#include <set>

AppConfig* AppConfig::get()
{
  static AppConfig* _inst = NULL;
  if(_inst == NULL)
    _inst = new AppConfig();
  return _inst;
}

AppConfig::AppConfig()
  : _log_level("info"),
    _app_home_dir("c:/ProgramData/GlassFlawDetect"),
    _data_save_dir("c:/ProgramData/GlassFlawDetect/data"),
    _save_orignal_images(false)
{
}

void AppConfig::load(const std::string& config_file_path)
{
  try {
    const auto data  = toml::parse(config_file_path);

    //plc
    _plc_id = toml::find<std::string>(data, "plc_id");
    _plc_port = toml::find<unsigned short>(data, "plc_port");

    //程序主目录
    _app_home_dir = toml::find<std::string>(data, "app_home_dir");
    if(_app_home_dir.empty())
      _app_home_dir = "c:/ProgramData/GlassFlawDetect";

    _data_save_dir = toml::find<std::string>(data, "data_save_dir"); //数据保存目录
    _mock_data_dir = toml::find<std::string>(data, "mock_data_dir"); //模拟运行数据目录

    _log_level = toml::find<std::string>(data, "log_level");
    {
      //校验
      std::set<std::string> levels = {"trace", "debug", "info", "warning", "error"};
      if(levels.find(_log_level) == levels.end()) {
        std::cout << "log_level is not valid! default set to \"info\"!" << std::endl;
        _log_level = "info";
      }
    }

    _save_orignal_images = toml::find<bool>(data, "save_orignal_images"); //是否保存原图片

    //图像拼合
    //拼合图片相比原始相机图片缩放比例(原始相机图片分辨率很大导致界面显示很慢)
    _face_image_combiner_scale_factor = toml::find<float>(data, "face_image_combiner_scale_factor");
    _edge_image_combiner_scale_factor = toml::find<float>(data, "edge_image_combiner_scale_factor");

    _front_image_combiner_stagger = toml::find<int>(data, "front_image_combiner_stagger");
    _back_image_combiner_stagger = toml::find<int>(data, "back_image_combiner_stagger");
    _top_image_combiner_stagger = toml::find<int>(data, "top_image_combiner_stagger");

  } catch(...) {
    std::cout << "failed to parse config file!" << std::endl;
    std::cout << "exit!" << std::endl;
    exit(1);
  }

  std::cout << "parse config file success!" << std::endl;
}
