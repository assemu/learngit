#ifndef V2_COMMON_H_
#define V2_COMMON_H_

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <core/util/json_util.h>
#include <core/plc/plc_config.h>

//相机工位
enum CameraPosition {
  EdgeRight,        //右边缘
  EdgeLeft,         //左边缘
  FrontLeft,        //正面左
  FrontRight,       //正面右
  BackLeft,         //背面左
  BackRight,        //背面右
  TopLeft,          //正上左，拍结石和气泡
  TopRight          //正上右
};

inline bool isEdgePosition(CameraPosition position)
{
  switch(position) {
    case EdgeRight:
    case EdgeLeft:
      return true;
    default:
      return false;
  }
  return false;
}

inline bool isFacePosition(CameraPosition position)
{
  switch(position) {
    case FrontLeft:
    case FrontRight:
    case BackLeft:
    case BackRight:
    case TopRight:
    case TopLeft:
      return true;
    default:
      return false;
  }
  return false;
}

inline std::vector<CameraPosition> allCameraPositions()
{
  std::vector<CameraPosition> re;
  re.push_back(EdgeRight);
  re.push_back(EdgeLeft);
  re.push_back(BackRight);
  re.push_back(BackLeft);
  re.push_back(FrontRight);
  re.push_back(FrontLeft);
  re.push_back(TopRight);
  re.push_back(TopLeft);
  return re;
}

inline CameraPosition positionOf(const std::string& camera_name)
{
  if(camera_name == "CAM3")
    return EdgeRight;
  else if(camera_name == "CAM4")
    return EdgeLeft;
  else if(camera_name == "CAM6")
    return FrontLeft;
  else if(camera_name == "CAM5")
    return FrontRight;
  else if(camera_name == "CAM8")
    return BackLeft;
  else if(camera_name == "CAM1")
    return TopLeft;
  else if(camera_name == "CAM2")
    return TopRight;
  else //(camera_name == "CAM7")
    return BackRight;
}

//相机工位对应的相机名称
inline std::string cameraNameOf(CameraPosition camera_position)
{
  switch(camera_position) {
    case EdgeRight:
      return "CAM3";
    case EdgeLeft:
      return "CAM4";
    case BackLeft:
      return "CAM8";
    case BackRight:
      return "CAM7";
    case FrontLeft:
      return "CAM6";
    case FrontRight:
      return "CAM5";
    case TopLeft:
      return "CAM1";
    case TopRight:
      return "CAM2";
  }
  return "";
}

struct CameraConfig {
  std::string config_file;          //相机配置文件
  int         image_steps;          //扫描图片张数
  int         analyze_begin;        //开始分析的图片序号
  int         analyze_end;          //结束分析的图片序号
  cv::Rect    edge_rois;            //edge roi配置
  float       face_binary_split_thresholds;                 //正面图像分割二值化阈值
  std::map<std::string, float> confidence_probabilities;    //每个分类的置信概率
  std::map<std::string, float> area_thresholds;             //每个分类的面积阈值
  std::map<std::string, float> length_thresholds;           //每个分类的长度阈值
  std::map<std::string, bool> label_filter_outs;            //每个分类是否过滤掉,true代表这个分类被过滤掉

  CameraConfig();

  //序列化和反序列化
  bool dumpToJsonStr(std::string& re) const;
  bool loadFromJsonStr(const std::string& json_str);
};

//玻璃型号以及其各种配置属性
struct GlassConfig {
  GlassConfig();

  std::string  name;                                        //型号名称
  bool filter_out_whole_bottom;                             //过滤掉整个底边（底边有水渍导致误检）
  std::map<CameraPosition, CameraConfig> camera_configs;    //相机配置,每个工位一个

  PlcConfigTrack track_config;      //运动轨迹配置
  PlcConfigRecipe recipe_config;    //配方配置

  //序列化和反序列化
  bool dumpToJsonStr(std::string& re) const;
  bool loadFromJsonStr(const std::string& json_str);
};

//批次
struct Batch {
  int64_t timestamp;
  std::string id;       //批次ID
  GlassConfig glass_config; //玻璃型号配置

  //序列化和反序列化
  bool dumpToJsonStr(std::string& re) const;
  bool loadFromJsonStr(const std::string& json_str);
};

//线扫描图片
struct PartImg {
  int64_t timestamp;        //时间戳
  CameraPosition position;  //相机工位
  cv::Mat img;              //线扫长条图,灰度图
  int part_num;             //序号
};

//分类和严重等级
//<label,level> 用于颜色等
inline std::map<std::string, int> allFlawLabels()
{
  std::map<std::string, int> labels;
  labels["white"] = 2;
  labels["black"] = 2;
  labels["bright"] = 2;
  labels["dark"] = 2;
  labels["scratch"] = 1;
  labels["dirty"] = 1;
  labels["circle"] = 0;
  //TODO 加入其它标签
  return labels;
}

/**
 * @brief 识别框信息
 */
struct BoxInfo {
  cv::Rect rect;        //识别框，相对于原图片
  float score;          //得分，即识别概率
  std::string label;    //标签
};

//缺陷
struct Flaw {
  int64_t timestamp;        //识别时间戳
  std::string id;           //缺陷唯一ID(在所属玻璃缺陷中)
  PartImg part_img;         //线扫描图，带序号
  cv::Mat detailed_img;     //缺陷细节图带识别框
  cv::Mat detailed_img_nomark;  //缺陷细节图不带识别框
  cv::Rect rect;            //在线扫描图中的位置
  float score;              //得分，置信概率
  std::string label;        //分类
  float area;               //实际面积值，单位平方毫米
  float realWidth;          //实际宽度,单位毫米
  float realHeight;         //实际长度,单位毫米

  inline int level() const
  {
    auto labels = allFlawLabels();
    if(labels.count(label) > 0)
      return labels[label];
    return 0;
  }
};

#endif //V2_COMMON_H_
