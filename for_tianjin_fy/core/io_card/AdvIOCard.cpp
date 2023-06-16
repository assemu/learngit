#include "AdvIOCard.h"
#include <iostream>
#include <core/util/log.h>

AdvIOCard::AdvIOCard()
{
  this->instantDiCtrl = InstantDiCtrl::Create();
  this->instantDoCtrl = InstantDoCtrl::Create();

  //std::cout << "IOCard:" << node.Description << std::endl
  //         << sptedDevicesI->getCount() << std::endl
  //        << node.DeviceNumber << std::endl;

  bool input_success = false;
  bool output_success = false;

  Array<DeviceTreeNode>* sptedDevicesI = this->instantDiCtrl->getSupportedDevices();
  for(int i = 0; i < sptedDevicesI->getCount(); i++) {
    DeviceTreeNode const& node = sptedDevicesI->getItem(i);
    auto ret = instantDiCtrl->setSelectedDevice(DeviceInformation(node.DeviceNumber));
    if(ret == Success) {
      input_success = true;
      break;
    }
  }

  if(!input_success)
    log_error("AdvIOCard input device init failed!");

  Array<DeviceTreeNode>* sptedDevicesO = this->instantDoCtrl->getSupportedDevices();
  for(int i = 0; i < sptedDevicesO->getCount(); i++) {
    DeviceTreeNode const& node = sptedDevicesO->getItem(i);
    ErrorCode ret = instantDoCtrl->setSelectedDevice(DeviceInformation(node.DeviceNumber));
    if(ret == Success) {
      output_success = true;
      break;
    }
  }

  if(!output_success)
    log_error("AdvIOCard output device init failed!");
}

AdvIOCard::~AdvIOCard()
{
  this->instantDiCtrl->Dispose();
  this->instantDoCtrl->Dispose();
}

bool AdvIOCard::readByte(WORD wPortNo, DWORD& dwOutValue)
{
  uint8 bDIVal = 0;
  ErrorCode ret = this->instantDiCtrl->Read((int32)wPortNo, bDIVal);
  if(ret == Success) {
    dwOutValue = bDIVal;
  }
  return (ret == Success);
}

bool AdvIOCard::readBit(WORD wPortNo, WORD wBitNo, WORD& wOutValue)
{
  uint8 bDIVal = 0;
  ErrorCode ret = this->instantDiCtrl->ReadBit((int32)wPortNo, (int32)wBitNo, &bDIVal);
  if(ret == Success) {
    wOutValue = (WORD)bDIVal;
  }
  return (ret == Success);
}

bool AdvIOCard::writeByte(DWORD wOutputPort, DWORD dwDOVal)
{
  ErrorCode ret = this->instantDoCtrl->Write(wOutputPort, dwDOVal);
  return (ret == Success);
}

bool AdvIOCard::writeBit(DWORD wOutputPort, WORD wBitNo, WORD wDOVal)
{
  ErrorCode ret = this->instantDoCtrl->WriteBit(wOutputPort, wBitNo, wDOVal);
  return (ret == Success);
}
