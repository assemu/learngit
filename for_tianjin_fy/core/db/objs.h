#ifndef V2_DAO_OBJS_H_
#define V2_DAO_OBJS_H_

#include <string>
#include <vector>
#include <core/common.h>
#include <core/util/json_util.h>
#include <core/util/log.h>

struct FlawRecord {
  int64_t id;
  int64_t timestamp;
  CameraPosition cam_position;   //工位
  std::string   glass_id;  //玻璃ID
  std::string   batch_id;  //批号ID
  std::string   label;     //分类
  float score;             //得分（置信概率）
  float area;              //面积
  float realWidth;         //真是宽度
  float realHeight;        //真实高度
  std::string detail_img_path;          //细节标注图片
  std::string detail_nomark_img_path;   //细节无标注图片
  bool audited; //是否已经审核
  bool audited_result; //审核结果
};

//单张玻璃统计
struct GlassStatisticsRecord {
  std::string glass_id;
  std::string glass_config;
  std::string batch_id;
  int white_count;
  int black_count;
  int bright_count;
  int dark_count;
  int dirty_count;
  int scratch_count;
};

//批次统计
struct BatchStatisticsRecord {
  std::string batch_id;
  int detect_count;
  int ok_count;
  int ng_count;

  BatchStatisticsRecord()
    : detect_count(0), ok_count(0), ng_count(0)
  {}

  //序列化
  bool dumpToJsonStr(std::string& re) const
  {
    try {
      auto root = nlohmann::json();
      root["batch_id"] = batch_id;
      root["detect_count"] = detect_count;
      root["ok_count"] = ok_count;
      root["ng_count"] = ng_count;
      re = root.dump(2);
      return true;
    } catch(...) {
      log_error("dump BatchStatisticsRecord to json failed!");
    }
    return false;
  }
};

//总检统计
struct AllStatisticsRecord {
  int detect_count;
  int ok_count;
  int ng_count;

  AllStatisticsRecord()
    : detect_count(0), ok_count(0), ng_count(0)
  {}

  //序列化
  bool dumpToJsonStr(std::string& re) const
  {
    try {
      auto root = nlohmann::json();
      root["detect_count"] = detect_count;
      root["ok_count"] = ok_count;
      root["ng_count"] = ng_count;
      re = root.dump(2);
      return true;
    } catch(...) {
      log_error("dump AllStatisticsRecord to json failed!");
    }
    return false;
  }
};

#endif //V2_DAO_OBJS_H_
