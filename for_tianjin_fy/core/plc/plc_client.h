#ifndef JIANFENG_CORE_PLC_CLIENT_H_
#define JIANFENG_CORE_PLC_CLIENT_H_

#include <string>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <windows.h>
#include <TcAdsDef.h>
#include <TcAdsAPI.h>
#include <core/util/log.h>
#include <core/util/str_util.h>

class PLCClient
{
public:
  static PLCClient* get();
  ~PLCClient();

public:
  //id
  inline std::string id() const
  {
    return _id;
  }

  //port
  unsigned short port() const
  {
    return _port;
  }

  bool isConnect();

  //设置回调
  //var_name 变量名称
  //circle_time 周期ms
  //cbLength 回调数据的size
  //callback 回调函数
  bool setNotificationCallBack(const std::string& var_name,
                               int cycle_time,
                               int cbLength,
                               std::function<void(void*)> callback);
  bool delNotificationCallBack(const std::string& var_name);

  //加载句柄
  bool loadPLCVarHandle(const std::string& var_name);

  //读取变量内容
  template<typename T>
  bool readVar(const std::string& var_name, T& v)
  {
    if(!loadPLCVarHandle(var_name)) {
      log_error("load plc var handle {} failed!", var_name);
      return false;
    }

    long nErr = AdsSyncReadReq(m_AdspAddr,
                               ADSIGRP_SYM_VALBYHND,
                               _handles[var_name],
                               sizeof(T),
                               &v);
    if(nErr) {
      log_error("read plc var {} failed! error:0x{:x}", var_name, nErr);
      return false;
    }

    log_trace("read plc var {} success!", var_name);
    return true;
  }

  //写入变量模板方法
  template<typename T>
  bool writeVar(const std::string& var_name, T v)
  {
    if(!loadPLCVarHandle(var_name)) {
      log_error("load plc var handle {} failed!", var_name);
      return false;
    }

    long nErr = AdsSyncWriteReq(m_AdspAddr,
                                ADSIGRP_SYM_VALBYHND,
                                _handles[var_name],
                                sizeof(T),
                                &v);
    if(nErr) {
      log_error("write plc var {} failed! error:0x{:x}", var_name, nErr);
      return false;
    } else {
      log_debug("write plc var {} success,value:{}!", var_name, v);
    }
    return true;
  }

  //写入字符串
  bool writeString(const std::string& var_name, const std::string& v);

  //写入数组变量
  bool writeVector(const std::string& var_name, const std::vector<double>& v);

private:
  PLCClient(const std::string& id, unsigned short port);

  std::string _id;
  unsigned short _port;
  PAmsAddr m_AdspAddr;

  std::map<std::string, unsigned long> _handles; //变量名称对应的句柄
};

#endif //JIANFENG_CORE_PLC_CLIENT_H_
