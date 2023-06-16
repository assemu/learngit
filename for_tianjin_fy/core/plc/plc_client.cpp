#include "plc_client.h"
#include <core/app_config.h>
#include <core/util/log.h>
#include <core/util/str_util.h>
#include <sstream>
#include <map>
#include <queue>

PLCClient* PLCClient::get()
{
  static PLCClient* _inst = NULL;
  if(_inst == NULL)
    _inst = new PLCClient(AppConfig::get()->getPLCID(), AppConfig::get()->getPLCPort());
  return _inst;
}

static void split(const std::string& s, std::vector<int>& sv, const char flag)
{
  sv.clear();
  std::istringstream iss(s);
  std::string temp;

  while(getline(iss, temp, flag)) {
    sv.push_back(stoi(temp));
  }
}

PLCClient::PLCClient(const std::string& id, unsigned short port)
  : _id(id),
    _port(port)
{
  m_AdspAddr = new AmsAddr();

  //什么作用
  long nPort = AdsPortOpen();

  std::vector<int> netvector;
  split(_id, netvector, '.');
  if(netvector.size() != sizeof(m_AdspAddr->netId.b)) {
    log_error("parse plc id error!");
    return;
  }

  //netid赋值
  for(int i = 0; i < sizeof(m_AdspAddr->netId.b); i++)
    m_AdspAddr->netId.b[i] = netvector[i];

  //端口
  m_AdspAddr->port = _port;
}

PLCClient::~PLCClient()
{
}

//写入字符串，将字符串的内存内容直接写入
bool PLCClient::writeString(const std::string& var_name, const std::string& v)
{
  if(!loadPLCVarHandle(var_name)) {
    log_error("load plc var handle {} failed!", var_name);
    return false;
  }

  long nErr = AdsSyncWriteReq(m_AdspAddr,
                              ADSIGRP_SYM_VALBYHND,
                              _handles[var_name],
                              v.length(),
                              (void*)v.c_str());
  if(nErr) {
    log_error("write plc var {} failed! error:0x{:x}", var_name, nErr);
    return false;
  }

  log_debug("write plc var {} success!", var_name);
  return true;
}

//写入数组，将vector的内存内容直接写入
bool PLCClient::writeVector(const std::string& var_name, const std::vector<double>& v)
{
  if(!loadPLCVarHandle(var_name)) {
    log_error("load plc var handle {} failed!", var_name);
    return false;
  }

  if(v.size() <= 0) {
    log_error("plc client write vector failed,vector is empty!");
    return false;
  }

  long nErr = AdsSyncWriteReq(m_AdspAddr,
                              ADSIGRP_SYM_VALBYHND,
                              _handles[var_name],
                              sizeof(double) * v.size(),
                              (void*)(&v[0]));
  if(nErr) {
    log_error("write plc var {} failed! error:0x{:x}", var_name, nErr);
    return false;
  }

  log_debug("write plc var {} success!", var_name);
  return true;
}

bool PLCClient::loadPLCVarHandle(const std::string& var_name)
{
  if(_handles.count(var_name) > 0)
    return true;

  unsigned long handle = 0;
  auto nErr = AdsSyncReadWriteReq(m_AdspAddr,
                                  ADSIGRP_SYM_HNDBYNAME,
                                  0x0,
                                  sizeof(handle), &handle,
                                  var_name.length(),
                                  (void*)var_name.c_str());
  if(nErr) {
    log_error("load plc handle of var {} failed! error:0x{:x}", var_name, nErr);
    return false;
  }

  log_debug("load plc handle of var {} success!", var_name);
  _handles[var_name] = handle;
  return true;
}

class CallbackCache
{
public:
  static CallbackCache* get()
  {
    static CallbackCache* _inst = NULL;
    if(_inst == NULL)
      _inst = new CallbackCache();
    return _inst;
  }

  //返回一个句柄
  unsigned long addCallback(const std::string& var_name, std::function<void(void*)> callback)
  {
    if(_queue.size() <= 0) {
      for(int i = 0; i < 1000; i++)
        _queue.push(++_index);
    }

    Item it;
    it.var_name = var_name;
    it.callback = callback;

    unsigned long handle = _queue.front();
    _queue.pop();
    _map[handle] = it;
    return handle;
  }

  std::function<void(void*)> getCallback(unsigned long handle)
  {
    if(_map.count(handle) > 0) {
      return _map[handle].callback;
    }

    log_error("PLCClient CallbackCache no callback for {}", handle);
    return [](void* data) { };
  }

  void removeCallback(const std::string& var_name)
  {
    auto it = _map.begin();
    while(it != _map.end()) {
      if((it->second).var_name == var_name) {
        _map.erase(it++);
        _queue.push(it->first);
      } else {
        ++it;
      }
    }
  }

private:
  struct Item {
    std::string var_name;
    std::function<void(void*)> callback;
  };

  CallbackCache()
  {
    _index = 0;
    for(int i = 0; i < 5000; i++)
      _queue.push(++_index);
  }

  std::map<unsigned long, Item> _map;
  std::queue<unsigned long> _queue;
  unsigned long _index;
};

void callback_proxy(AmsAddr*	pAddr, AdsNotificationHeader* pNotification, unsigned long hUser)
{
  if(pNotification == NULL)
    return;

  void* data = (void*)(pNotification->data);
  auto callback = CallbackCache::get()->getCallback(hUser);
  callback(data);
}

static std::string notification_handle_key_of(const std::string& var_name)
{
  std::stringstream ss;
  ss << var_name << ".___NOTIFICATION__";
  return ss.str();
}

bool PLCClient::setNotificationCallBack(const std::string& var_name,
                                        int cycle_time,
                                        int cbLength,
                                        std::function<void(void*)> callback)
{
  if(!loadPLCVarHandle(var_name)) {
    log_error("load plc var handle {} failed!", var_name);
    return false;
  }

  AdsNotificationAttrib attr;
  attr.cbLength = cbLength; //?
  attr.nTransMode = ADSTRANS_SERVERONCHA;
  attr.nMaxDelay = 0;
  attr.nCycleTime = cycle_time;

  //无法用指针地址转换成unsigned long 会截断
  //所以这里使用了一个unsigned long 到回调函数的映射方法
  unsigned long c_handle = CallbackCache::get()->addCallback(var_name, callback);

  unsigned long notification_handle;
  long nErr = AdsSyncAddDeviceNotificationReq(
                m_AdspAddr,
                ADSIGRP_SYM_VALBYHND,
                _handles[var_name],
                &attr,
                (PAdsNotificationFuncEx)callback_proxy,     //一个代理函数
                c_handle,
                &notification_handle);
  if(nErr) {
    log_error("add device notification {} failed! error:0x{:x}", var_name, nErr);
    return false;
  }

  log_debug("add device notification {} success!", var_name);
  auto key = notification_handle_key_of(var_name);
  _handles[key] = notification_handle;
  return true;
}

bool PLCClient::delNotificationCallBack(const std::string& var_name)
{
  auto key = notification_handle_key_of(var_name);
  if(_handles.count(key) > 0) {
    AdsSyncDelDeviceNotificationReq(m_AdspAddr, _handles[key]);
    _handles.erase(key);
    CallbackCache::get()->removeCallback(var_name);
    return true;
  }

  return false;
}

bool PLCClient::isConnect()
{
  return loadPLCVarHandle("GVL_AQ.fenduan_1");
}
