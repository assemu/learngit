#include "dalsa_camera.h"
#include "camera_part_img_source.h"
#include <core/util/time_util.h>

DalsaCamera::DalsaCamera(CameraPosition camera_position, CameraPartImgSource* supplier)
  : _camera_position(camera_position), _supplier(supplier)
{
  m_AcqDevice = NULL;
  m_Buffers = NULL;
  m_Xfer = NULL;
  ///
  m_Feature = NULL;

  serverCount = SapManager::GetServerCount();
  std::cout << "serverCount " << serverCount << std::endl;
  if(serverCount == 0) {
    std::cout << "No camera device found" << std::endl;
  }

  for(int i = 0; i < serverCount; i++) {
    //GetResourceCount校验是否有AcqDevice设备
    int resourceCount = SapManager::GetResourceCount(i, SapManager::ResourceAcqDevice);
    if(resourceCount != 0) {
      SapManager::GetServerName(i, acqServerName, sizeof(acqServerName)); //GetServerName获取相机名称

      //查找相机编号
      char acqcameraName[CORPRM_GETSIZE(CORACQ_PRM_LABEL)];
      if(SapManager::GetResourceName(acqServerName, SapManager::ResourceAcqDevice, 0, acqcameraName, sizeof(acqcameraName))) {
        if(!strcmp(acqcameraName, cameraName().c_str())) {
          std::cout << "GetResourceName cameraName = " << cameraName() << ", acqServerName = " << acqServerName << std::endl;
          break;
        }
      }
    }
  }
}

DalsaCamera::~DalsaCamera()
{
  if(m_Xfer)
    delete m_Xfer;

  if(m_Buffers)
    delete m_Buffers;

  if(m_AcqDevice)
    delete m_AcqDevice;
  ///
  if(m_Feature)
    delete m_Feature;
}

bool DalsaCamera::cameraInit(const std::string& config_file_path)
{
  std::cout << "init camera " << cameraName() << " with " << config_file_path << std::endl;

  if(serverCount == 0)
    return false;

  SapLocation loc(acqServerName, 0);
  m_AcqDevice = new SapAcqDevice(loc, config_file_path.c_str());
  if(!m_AcqDevice)
    return false;

  m_Buffers = new SapBufferWithTrash(3, m_AcqDevice);
  if(!m_Buffers)
    return false;

  m_Xfer = new SapAcqDeviceToBuf(m_AcqDevice, m_Buffers, XferCallback, this);
  if(!m_Xfer)
    return false;
  ///
  m_Feature = new SapFeature(loc);
  if(!m_Feature)
    return false;

  // Create acquisition object
  if(m_AcqDevice && !*m_AcqDevice && !m_AcqDevice->Create()) {
    cameraDestroy();
    return false;
  }

  // Create buffer object
  if(m_Buffers && !*m_Buffers) {
    if(!m_Buffers->Create()) {
      cameraDestroy();
      return false;
    }
    // Clear all buffers
    m_Buffers->Clear();
  }

  // Set next empty with trash cycle mode for transfer
  if(m_Xfer && m_Xfer->GetPair(0)) {
    if(!m_Xfer->GetPair(0)->SetCycleMode(SapXferPair::CycleNextWithTrash)) {
      cameraDestroy();
      return false;
    }
  }

  // Create transfer object
  if(m_Xfer && !*m_Xfer && !m_Xfer->Create()) {
    cameraDestroy();
    return false;
  }

  ///
  // Create feature object
  if(m_Feature && !*m_Feature && !m_Feature->Create()) {
    cameraDestroy();
    return false;
  }


  std::cout << "success!" << std::endl;
  return true;
}

bool DalsaCamera::isCameraInited()
{
  return m_AcqDevice != NULL;
}

void DalsaCamera::XferCallback(SapXferCallbackInfo* pInfo)
{
  DalsaCamera* self = (DalsaCamera*)pInfo->GetContext();
  if(pInfo->IsTrash()) {
    //self->m_buffer->m_cfg.getProPtr()->eventsLogFile << Debug::logPrefix("XferCallback 触发相机 " + self->no) << std::endl;
    //std::cout << "Frames acquired in trash buffer:" << pInfo->GetEventCount() << std::endl;
  } else {
    /*pInfo->GetTransfer()->Grab()*/
    BYTE pData;
    void* pDataAddr = &pData;
    BOOL success = self->m_Buffers->GetAddress(&pDataAddr);
    if(success) {
      int	 width = self->m_Buffers->GetWidth();
      int  height = self->m_Buffers->GetHeight();
      cv::Mat img = cv::Mat::zeros(cv::Size(width, height), CV_8U); //灰度图
      memcpy(img.data, pDataAddr, width * height); //拷贝图像

      if(!img.empty()) {
        PartImg pimg;
        pimg.timestamp = get_timestamp();         //时间戳
        pimg.position = self->cameraPosition();   //工位
        pimg.img = img;                           //线扫长条图
        self->_pushToSupplier(pimg);
      }
    }
  }
}

void DalsaCamera::_pushToSupplier(const PartImg& img)
{
  _supplier->_pushImg(img);
}

void DalsaCamera::startGrab()
{
  if(m_Xfer && ! m_Xfer->IsGrabbing()) {
    m_Xfer->Grab();
  }
  //if (m_Xfer->Grab())
  //{
  //}
}

void DalsaCamera::stopGrab()
{
  if(m_Xfer && m_Xfer->IsGrabbing()) {
    m_Xfer->Freeze();
  }
}

void DalsaCamera::cameraDestroy()
{
  stopGrab();

  // Destroy transfer object
  if(m_Xfer && *m_Xfer)
    m_Xfer->Destroy();

  // Destroy buffer object
  if(m_Buffers && *m_Buffers)
    m_Buffers->Destroy();

  // Destroy acquisition object
  if(m_AcqDevice && *m_AcqDevice)
    m_AcqDevice->Destroy();

  ///
  // Destroy feature object
  if(m_Feature && !*m_Feature)
    m_Feature->Destroy();
}
