#ifndef V2_PLC_CONFIG_H__
#define V2_PLC_CONFIG_H__

#include <string>
#include <vector>
#include <core/util/json_util.h>
#include <core/util/log.h>

//PLC配置基类
class PlcConfigBase
{
public:
  //获取配置项
  template <typename T>
  bool getConfigItem(const std::string& key, T& re) const
  {
    try {
      re = root[key].get<T>();
      return true;
    } catch(...) { }
    return false;
  }

  //设置配置项
  template <typename T>
  bool setConfigItem(const std::string& key, const T& v)
  {
    try {
      root[key] = v;
      return true;
    } catch(...) { }
    return false;
  }

  //序列化和反序列化
  bool dumpToJsonStr(std::string& re) const
  {
    try {
      re = root.dump(2);
      return true;
    } catch(...) {
      log_error("dump PlcConfig to json failed!");
    }
    return false;
  }

  bool loadFromJsonStr(const std::string& json_str)
  {
    try {
      root = nlohmann::json::parse(json_str);
      return true;
    } catch(...) {
      log_error("load PlcConfig from json failed!");
    }
    return false;
  }

protected:
  nlohmann::json root;
};

//PLC运动轨迹配置
class PlcConfigTrack : public PlcConfigBase
{
public:
  PlcConfigTrack();

  //获取轨迹数据
  bool getCAMTrackData(const std::string& key, std::vector<double>& data) const
  {
    try {
      auto node = root[key];
      if(node.is_array()) {
        data.clear();
        for(auto& j : node) {
          data.push_back(j.get<double>());
        }
        return true;
      }
    } catch(...) {}
    return false;
  }

  //设置轨迹数据
  bool setCAMTrackData(const std::string& key, const std::vector<double>& data)
  {
    if(!key.empty()) {
      try {
        auto j = nlohmann::json::array();
        for(double d : data)
          j.push_back(d);
        root[key] = j;
        return true;
      } catch(...) {}
    }
    return false;
  }
};

//PLC配方配置
class PlcConfigRecipe : public PlcConfigBase
{
public:
  PlcConfigRecipe();
};

#endif //V2_PLC_CONFIG_H__
