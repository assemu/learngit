#ifndef J_APP_CONFIG_H_
#define J_APP_CONFIG_H_

#include <string>
#include <QString>
#include <map>
#include <opencv2/opencv.hpp>

class AppConfig
{
public:
  static AppConfig* get();

  //加载配置文件
  void load(const std::string& config_file_path);

  //plc ID
  inline std::string getPLCID() const
  {
    return _plc_id;
  }

  //plc port
  inline unsigned short getPLCPort() const
  {
    return _plc_port;
  }

  //模拟数据文件夹
  std::string getMockDataDir() const
  {
    return QString::fromStdString(_mock_data_dir).replace("\\", "/").toStdString();
  }

  //获取数据保存目录
  inline std::string getDataSaveDir() const
  {
    return QString::fromStdString(_data_save_dir).replace("\\", "/").toStdString();
  }

  //获取程序主目录
  inline std::string getAppHomeDir() const
  {
    return QString::fromStdString(_app_home_dir).replace("\\", "/").toStdString();
  }

  //获取日志记录级别
  inline std::string getLogLevel() const
  {
    return _log_level;
  }

  //是否保存原图
  inline bool isSaveOrignalImages() const
  {
    return _save_orignal_images;
  }
  inline void setSaveOrignalImages(bool v)
  {
    _save_orignal_images = v;
  }

  //面图像拼合缩放因子
  inline float getFaceImageCombinerScaleFactor() const
  {
    return _face_image_combiner_scale_factor;
  }

  //面图像拼合错位
  inline float getFrontImageCombinerStagger() const
  {
    return _front_image_combiner_stagger;
  }

  inline float getBackImageCombinerStagger() const
  {
    return _back_image_combiner_stagger;
  }

  inline float getTopImageCombinerStagger() const
  {
    return _top_image_combiner_stagger;
  }

  inline float getEdgeImageCombinerScaleFactor() const
  {
    return _edge_image_combiner_scale_factor;
  }

private:
  AppConfig();

  //目录
  std::string _app_home_dir;
  std::string _mock_data_dir;
  std::string _data_save_dir;

  //是否保存原图
  bool _save_orignal_images;

  //图像拼合
  float _face_image_combiner_scale_factor;
  float _edge_image_combiner_scale_factor;
  float _front_image_combiner_stagger;
  float _back_image_combiner_stagger;
  float _top_image_combiner_stagger;

  //日志级别
  std::string _log_level;

  //plc
  std::string _plc_id;
  unsigned short _plc_port;
};

#endif //J_APP_CONFIG_H_
