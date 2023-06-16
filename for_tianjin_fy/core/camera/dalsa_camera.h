#ifndef __DALSA__CAMERA_API_H_
#define __DALSA__CAMERA_API_H_

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <conio.h>
#include <math.h>
#include <sapclassbasic.h>
#include <opencv2/opencv.hpp>
#include <core/common.h>

class CameraPartImgSource;

class DalsaCamera
{
public:
  DalsaCamera(CameraPosition camera_position, CameraPartImgSource* supplier);
  ~DalsaCamera();

  bool cameraInit(const std::string& config_file_path);
  bool isCameraInited();
  void cameraDestroy();

  void startGrab();
  void stopGrab();

  //参数设置,call after cameraInit()
  BOOL GetParameter(int param, void* pValue)
  {
    if(m_AcqDevice != NULL)
      return m_AcqDevice->GetParameter(param, pValue);
    return false;
  }

  BOOL SetParameter(int param, int value)
  {
    if(m_AcqDevice != NULL)
      return m_AcqDevice->SetParameter(param, value);
    return false;
  }

  BOOL SetParameter(int param, void* pValue)
  {
    if(m_AcqDevice != NULL)
      return m_AcqDevice->SetParameter(param, pValue);
    return false;
  }
  ///
  // Feature 参数获取 FredrickLee
  BOOL GetFeature(const char* feature_name, double* pValue)
  {
	  BOOL flag = false;
	  if (m_AcqDevice->IsFeatureAvailable(feature_name, &flag))
	  {
		  std::cout << "Get Feature=====>" << feature_name << std::endl;
		  return m_AcqDevice->GetFeatureValue(feature_name, pValue);

	  }
	  return false;
  }
  BOOL GetFeature(const char* feature_name, int* pValue)
  {
	  BOOL flag = false;
	  if (m_AcqDevice->IsFeatureAvailable(feature_name, &flag))
	  {
		  std::cout << "Get Feature=====>" << feature_name << std::endl;
		  return m_AcqDevice->GetFeatureValue(feature_name, pValue);

	  }
	  return false;
  }

  BOOL SetFeature(const char* feature_name, double value)
  {
	  BOOL flag = false;
	  if (m_AcqDevice->IsFeatureAvailable(feature_name, &flag))
	  {
		  std::cout << "Set Feature=====>" << feature_name << " " << value << std::endl;
		  return m_AcqDevice->SetFeatureValue(feature_name, value);

	  }
	  return false;
  }
  //相机工位
  inline CameraPosition cameraPosition() const
  {
    return _camera_position;
  }

  inline std::string cameraName() const
  {
    return cameraNameOf(_camera_position);
  }

  void _pushToSupplier(const PartImg& img);

public:
  int serverCount;
  SapBufferWithTrash*   m_Buffers;
  static void XferCallback(SapXferCallbackInfo* pInfo);

private:
  char acqServerName[CORSERVER_MAX_STRLEN];

  SapAcqDevice*         m_AcqDevice;
  SapTransfer*          m_Xfer;
  //FredrickLee m_Feature related 2022 12 03
  SapFeature*			m_Feature;

  CameraPosition _camera_position; //相机工位
  CameraPartImgSource* _supplier;
};

#endif //__DALSA__CAMERA_API_H_ 
