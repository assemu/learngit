#include "common.h"
#include <core/util/json_util.h>
#include <core/util/log.h>

using json = nlohmann::json;

CameraConfig::CameraConfig()
{
  config_file = "";
  image_steps = 64;
  analyze_begin = 1;
  analyze_end = 60;
  edge_rois = cv::Rect(880, 0, 500, 500);
  face_binary_split_thresholds = 30.0;

  for(auto kv : allFlawLabels()) {
    auto label = kv.first;
    confidence_probabilities[label] = 0.1;
    area_thresholds[label] = 0.1;
    length_thresholds[label] = 0.1;
    label_filter_outs[label] = false;
  }
}

bool CameraConfig::dumpToJsonStr(std::string& re) const
{
  try {
    auto root = json();
    root["config_file"] = config_file;
    root["image_steps"] = image_steps;
    root["analyze_begin"] = analyze_begin;
    root["analyze_end"] = analyze_end;
    root["face_binary_split_thresholds"] = face_binary_split_thresholds;
    {
      auto j = json::array();
      j.push_back(edge_rois.x);
      j.push_back(edge_rois.y);
      j.push_back(edge_rois.width);
      j.push_back(edge_rois.height);
      root["edge_rois"] = j;
    }

    {
      auto j = json();
      for(auto& kv : confidence_probabilities)
        j[kv.first] = kv.second;
      root["confidence_probabilities"] = j;
    }

    {
      auto j = json();
      for(auto& kv : area_thresholds)
        j[kv.first] = kv.second;
      root["area_thresholds"] = j;
    }

    {
      auto j = json();
      for(auto& kv : length_thresholds)
        j[kv.first] = kv.second;
      root["length_thresholds"] = j;
    }

    {
      auto j = json();
      for(auto& kv : label_filter_outs)
        j[kv.first] = kv.second;
      root["label_filter_outs"] = j;
    }

    re = root.dump(2);
    return true;
  } catch(...) {
    log_error("dump CameraConfig to json failed!");
  }
  return false;
}

bool CameraConfig::loadFromJsonStr(const std::string& json_str)
{
  try {
    auto root = json::parse(json_str);
    this->config_file = root["config_file"].get<std::string>();
    this->image_steps = root["image_steps"].get<int>();
    this->analyze_begin = root["analyze_begin"].get<int>();
    this->analyze_end = root["analyze_end"].get<int>();
    this->face_binary_split_thresholds = root["face_binary_split_thresholds"].get<float>();
    {
      int v[4];
      auto j = root["edge_rois"];
      for(int i = 0; i < 4; i++)
        v[i] = j.at(i).get<int>();
      this->edge_rois = cv::Rect(v[0], v[1], v[2], v[3]);
    }

    this->confidence_probabilities.clear();
    for(auto& j : root["confidence_probabilities"].items()) {
      auto label = j.key();
      auto v = j.value().get<float>();
      this->confidence_probabilities[label] = v;
    }

    this->area_thresholds.clear();
    for(auto& j : root["area_thresholds"].items()) {
      auto label = j.key();
      auto v = j.value().get<float>();
      this->area_thresholds[label] = v;
    }

    this->length_thresholds.clear();
    for(auto& j : root["length_thresholds"].items()) {
      auto label = j.key();
      auto v = j.value().get<float>();
      this->length_thresholds[label] = v;
    }

    this->label_filter_outs.clear();
    for(auto& j : root["label_filter_outs"].items()) {
      auto label = j.key();
      auto v = j.value().get<bool>();
      this->label_filter_outs[label] = v;
    }

    return true;
  } catch(...) {
    log_error("load CameraConfig from json failed!");
  }
  return false;
}

bool GlassConfig::dumpToJsonStr(std::string& re) const
{
  try {
    auto root = json();
    root["name"] = name;
    root["filter_out_whole_bottom"] = filter_out_whole_bottom;

    {
      auto j = json();
      for(auto& kv : camera_configs) {
        auto key = cameraNameOf(kv.first);
        auto c = kv.second;
        std::string json_str;
        c.dumpToJsonStr(json_str);
        auto cj = json::parse(json_str);
        j[key] = cj;
      }
      root["camera_configs"] = j;
    }

    {
      std::string json_str;
      track_config.dumpToJsonStr(json_str);
      auto j = json::parse(json_str);
      root["track_config"] = j;
    }

    {
      std::string json_str;
      recipe_config.dumpToJsonStr(json_str);
      auto j = json::parse(json_str);
      root["recipe_config"] = j;
    }

    re = root.dump(2);
    return true;
  } catch(...) {
    log_error("dump GlassConfig to json failed!");
  }
  return false;
}

bool GlassConfig::loadFromJsonStr(const std::string& json_str)
{
  try {
    auto root = json::parse(json_str);
    this->name = root["name"].get<std::string>();
    this->filter_out_whole_bottom = root["filter_out_whole_bottom"].get<bool>();

    this->camera_configs.clear();
    for(auto& j : root["camera_configs"].items()) {
      auto cam_name = j.key();
      CameraConfig c;
      c.loadFromJsonStr(j.value().dump(2));
      this->camera_configs[positionOf(cam_name)] = c;
    }

    {
      auto j = root["track_config"];
      this->track_config.loadFromJsonStr(j.dump(2));
    }

    {
      auto j = root["recipe_config"];
      this->recipe_config.loadFromJsonStr(j.dump(2));
    }

    return true;
  } catch(...) {
    log_error("load GlassConfig from json failed!");
  }
  return false;
}

bool Batch::dumpToJsonStr(std::string& re) const
{
  try {
    auto root = json();
    root["id"] = id;
    root["timestamp"] = timestamp;
    {
      std::string json_str;
      glass_config.dumpToJsonStr(json_str);
      root["glass_config"] = json::parse(json_str);
    }
    re = root.dump(2);
    return true;
  } catch(...) {
    log_error("dump Batch to json failed!");
  }
  return false;
}

bool Batch::loadFromJsonStr(const std::string& json_str)
{
  try {
    auto root = json::parse(json_str);
    this->id = root["id"].get<std::string>();
    this->timestamp = root["timestamp"].get<int64_t>();
    (this->glass_config).loadFromJsonStr(root["glass_config"].dump(2));
    return true;
  } catch(...) {
    log_error("load Batch from json failed!");
  }
  return false;
}

GlassConfig::GlassConfig()
{
  name = "";
  filter_out_whole_bottom = false;
  for(auto& p : allCameraPositions()) {
    camera_configs[p] = CameraConfig();
  }

  camera_configs[EdgeRight].config_file = "F:/cameraConfig_20220423/CAM3/T_Linea_M2048-7um_Default_Default.ccf";
  camera_configs[EdgeLeft].config_file = "F:/cameraConfig_20220423/CAM4/T_Linea_M2048-7um_Default_Default.ccf";
  camera_configs[FrontRight].config_file = "F:/cameraConfig_20220423/CAM5/T_Linea_M8192-7um_Default_Default.ccf";
  camera_configs[FrontLeft].config_file = "F:/cameraConfig_20220423/CAM6/T_Linea_M8192-7um_Default_Default.ccf";
  camera_configs[BackRight].config_file = "F:/cameraConfig_20220423/CAM7/T_Linea_M8192-7um_Default_Default.ccf";
  camera_configs[BackLeft].config_file = "F:/cameraConfig_20220423/CAM8/T_Linea_M8192-7um_Default_Default.ccf";
  camera_configs[TopLeft].config_file = "F:/cameraConfig_20220423/CAM1/T_Linea_M8192-7um_Default_Default.ccf";
  camera_configs[TopRight].config_file = "F:/cameraConfig_20220423/CAM2/T_Linea_M8192-7um_Default_Default.ccf";
}
