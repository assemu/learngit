#ifndef V2_CAMERA_PART_IMG_SOURCE_H_
#define V2_CAMERA_PART_IMG_SOURCE_H_

#include <memory>
#include <map>
#include <QList>
#include <QMutex>
#include <core/common.h>
#include <core/part_img_source.h>

class DalsaCamera;

//线扫图像采集器
//封装8个camera
class CameraPartImgSource : public PartImgSource
{
public:
  static CameraPartImgSource* get();
  ~CameraPartImgSource();

  //开始采集
  virtual bool startWork(GlassConfig glass_config) override;

  //停止采集
  virtual bool stopWork() override;

  //注册图像处理
  virtual void registerPartImgConsumer(PartImgConsumer* consumer) override;

  //注册图像处理对象
  virtual void unregisterPartImgConsumer(PartImgConsumer* consumer) override;

public:
  //相机参数设置相关方法
  //曝光延迟时间
  bool GetParameterTriggerDelayMin(CameraPosition position, void* pValue);  //参数最小值
  bool GetParameterTriggerDelayMax(CameraPosition position, void* pValue);  //参数最大值
  bool SetParameterTriggerDelay(CameraPosition position, int value);
  bool GetParameterTriggerDelay(CameraPosition position, void* pValue);

  ///
  bool GetFeatureExposureTime(CameraPosition position, double* pValue);
  bool SetFeatureExposureTime(CameraPosition position, double value);
  //增益
  //bool GetParameterGainMin(CameraPosition position, void* pValue);
  //bool GetParameterGainMax(CameraPosition position, void* pValue);
  bool GetFeatureGain(CameraPosition position, double* pValue);
  bool SetFeatureGain(CameraPosition position, double value);

  //行频
  //bool GetParameterRowFreqMin(CameraPosition position, void* pValue);
  //bool GetParameterRowFreqMax(CameraPosition position, void* pValue);
  bool GetFeatureLineRate(CameraPosition position, double* pValue);
  bool SetFeatureLineRate(CameraPosition position, double value);

  //单帧行数
  //bool GetParameterFrameRateMin(CameraPosition position, void* pValue);
  //bool GetParameterFrameRateMax(CameraPosition position, void* pValue);
  bool GetFeatureLineCount(CameraPosition position, int* pValue);
  bool SetFeatureLineCount(CameraPosition position, int value);

  //帧数
  bool GetFeatureFrameCount(CameraPosition position, int* pValue);
  bool SetFeatureFrameCount(CameraPosition position, int value);

  bool SetParameterFrameRate(CameraPosition position, int value);

  //IO
  //bool GetParameterIOMin(CameraPosition position, void* pValue);
  //bool GetParameterIOMax(CameraPosition position, void* pValue);
  bool GetParameterIO(CameraPosition position, void* pValue);
  bool SetParameterIO(CameraPosition position, int value);

public:
  //called in camera threads
  void _pushImg(const PartImg& img);

private:
  CameraPartImgSource();

  bool GetParameter(CameraPosition position, int param, void* pValue);
  bool SetParameter(CameraPosition position, int param, int value);
  bool SetParameter(CameraPosition position, int param, void* pValue);

  std::shared_ptr<DalsaCamera> _camera(CameraPosition position);
  QList<std::shared_ptr<DalsaCamera>> _cameras;
  QList<PartImgConsumer*> _consumers;
  QMutex _consumers_mutex;
};

#endif //V2_CAMERA_PART_IMG_SOURCE_H_
