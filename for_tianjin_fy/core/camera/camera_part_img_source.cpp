#include "camera_part_img_source.h"
#include "dalsa_camera.h"
#include <iostream>
#include <core/part_img_consumer.h>
#include <core/util/log.h>
#include <QMutexLocker>

CameraPartImgSource* CameraPartImgSource::get()
{
  static CameraPartImgSource* _inst = NULL;
  if(_inst == NULL)
    _inst = new CameraPartImgSource();
  return _inst;
}

CameraPartImgSource::CameraPartImgSource()
{
  _cameras << std::make_shared<DalsaCamera>(EdgeLeft, this);
  _cameras << std::make_shared<DalsaCamera>(EdgeRight, this);
  _cameras << std::make_shared<DalsaCamera>(FrontRight, this);
  _cameras << std::make_shared<DalsaCamera>(FrontLeft, this);
  _cameras << std::make_shared<DalsaCamera>(BackLeft, this);
  _cameras << std::make_shared<DalsaCamera>(BackRight, this);
  _cameras << std::make_shared<DalsaCamera>(TopLeft, this);
  _cameras << std::make_shared<DalsaCamera>(TopRight, this);
}

CameraPartImgSource::~CameraPartImgSource()
{
  stopWork();
}

bool CameraPartImgSource::startWork(GlassConfig glass_config)
{
  //使用型号配置中的相机配置文件初始化
  for(auto c : _cameras) {
    //if (!c->isCameraInited()) {
    auto position = c->cameraPosition();
    if(glass_config.camera_configs.count(position) > 0) {
      auto config_file_path = glass_config.camera_configs[position].config_file;
      c->cameraInit(config_file_path);
    } else {
      log_error("init camera:{} failed! no config file provided!", cameraNameOf(position));
    }
    //}
  }

  //启动相机采集
  for(auto& camera : _cameras)
    camera->startGrab();

  return true;
}

bool CameraPartImgSource::stopWork()
{
  //停止采集
  for(auto& camera : _cameras) {
    log_info("camera:{} stopGrab", cameraNameOf(camera->cameraPosition()));
    camera->stopGrab();
  }

  //销毁
  for(auto& camera : _cameras) {
    log_info("camera:{} cameraDestroy", cameraNameOf(camera->cameraPosition()));
    camera->cameraDestroy();
  }

  return true;
}

void CameraPartImgSource::registerPartImgConsumer(PartImgConsumer* consumer)
{
  QMutexLocker locker(&_consumers_mutex);
  if(consumer)
    _consumers.append(consumer);
}

void CameraPartImgSource::unregisterPartImgConsumer(PartImgConsumer* consumer)
{
  QMutexLocker locker(&_consumers_mutex);
  _consumers.removeAll(consumer);
}

void CameraPartImgSource::_pushImg(const PartImg& img)
{
  for(auto& consumer : _consumers) {
    if(isEdgePosition(img.position))
      consumer->acceptEdgePartImg(img);
    else
      consumer->acceptFacePartImg(img);
  }
}

std::shared_ptr<DalsaCamera> CameraPartImgSource::_camera(CameraPosition position)
{
  for(auto c : _cameras) {
    auto p = c->cameraPosition();
    if(p == position)
      return c;
  }
  return NULL;
}

bool CameraPartImgSource::GetParameter(CameraPosition position, int param, void* pValue)
{
  auto c = _camera(position);
  if(c)
    return c->GetParameter(param, pValue);
  return false;
}

bool CameraPartImgSource::SetParameter(CameraPosition position, int param, int value)
{
  auto c = _camera(position);
  if(c)
    return c->SetParameter(param, value);
  return false;
}

bool CameraPartImgSource::SetParameter(CameraPosition position, int param, void* pValue)
{
  auto c = _camera(position);
  if(c)
    return c->SetParameter(param, pValue);
  return false;
}

bool CameraPartImgSource::GetParameterTriggerDelay(CameraPosition position, void* pValue)
{
  *((int*)pValue) = 1000;

  //TODO
  //GetParameter(position,);
  return false;
}

bool CameraPartImgSource::GetParameterTriggerDelayMin(CameraPosition position, void* pValue)
{
  *((int*)pValue) = 100;
  //TODO
  return false;
}

bool CameraPartImgSource::GetParameterTriggerDelayMax(CameraPosition position, void* pValue)
{
  *((int*)pValue) = 10000;
  //TODO
  return false;
}

bool CameraPartImgSource::SetParameterTriggerDelay(CameraPosition position, int value)
{
  //TODO
  return false;
}

///
//曝光
bool CameraPartImgSource::GetFeatureExposureTime(CameraPosition position, double* pValue)
{
  auto c = _camera(position);
  if(c)
    return c->GetFeature("ExposureTime", pValue);
  return false;
}
bool CameraPartImgSource::SetFeatureExposureTime(CameraPosition position, double value)
{
  auto c = _camera(position);
  if(c)
    return c->SetFeature("ExposureTime", value);
  return false;
}
//增益
bool CameraPartImgSource::GetFeatureGain(CameraPosition position, double* pValue)
{
  //TODO
  auto c = _camera(position);
  if(c)
    return c->GetFeature("Gain", pValue);
  return false;
}

bool CameraPartImgSource::SetFeatureGain(CameraPosition position, double value)
{
  //TODO
  auto c = _camera(position);
  if(c)
    return c->SetFeature("Gain", value);
  return false;
}


//行频 AcqLineRate
bool CameraPartImgSource::GetFeatureLineRate(CameraPosition position, double* pValue)
{
  //TODO
  auto c = _camera(position);
  if(c)
    return c->GetFeature("AcquisitionLineRate", pValue);
  return false;
}

bool CameraPartImgSource::SetFeatureLineRate(CameraPosition position, double value)
{
  //TODO
  auto c = _camera(position);
  if(c)
    return c->SetFeature("AcquisitionLineRate", value);
  return false;
}

//帧数
bool CameraPartImgSource::GetFeatureFrameCount(CameraPosition position, int* pValue)
{
  auto c = _camera(position);
  if(c)
    return c->GetFeature("triggerFrameCount", pValue);
  return false;
}
bool CameraPartImgSource::SetFeatureFrameCount(CameraPosition position, int value)
{
  auto c = _camera(position);
  if(c)
    return c->SetFeature("triggerFrameCount", value);
  return false;
}

//单帧行数
bool CameraPartImgSource::GetFeatureLineCount(CameraPosition position, int* pValue)
{
  //TODO
  auto c = _camera(position);
  if(c)
    return c->GetFeature("Height", pValue);
  return false;
}
bool CameraPartImgSource::SetFeatureLineCount(CameraPosition position, int value)
{
  // 不做动作，防止软件因修改1000 SIZE而崩溃
  //auto c = _camera(position);
  //if (c)
  //  return c->SetFeature("Height", value);
  //return false;
  return false;
}




bool CameraPartImgSource::SetParameterFrameRate(CameraPosition position, int value)
{
  //TODO CORACQ_PRM_EXT_TRIGGER_FRAME_COUNT
  SetParameter(position, CORACQ_PRM_EXT_TRIGGER_FRAME_COUNT, value);
  return false;
}


//IO
bool CameraPartImgSource::GetParameterIO(CameraPosition position, void* pValue)
{
  //TODO
  return false;
}

bool CameraPartImgSource::SetParameterIO(CameraPosition position, int value)
{
  //TODO
  return false;
}
