#include "account_manager.h"
#include <core/util/str_util.h>
#include <core/util/filesystem_util.h>
#include <core/util/log.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <core/util/json_util.h>
#include <core/util/hash_util.h>
#include <core/util/time_util.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <core/app_config.h>

using json = nlohmann::json;

class AccountManagerPrivate
{
public:
  std::shared_ptr<Account> _current;
  std::string _account_config_file_path;
  json root;

  AccountManagerPrivate()
  {
    _current = NULL;
    _account_config_file_path = AppConfig::get()->getAppHomeDir() + "/accounts.json";
    _load();
  }

  bool _load()
  {
    if(!boost::filesystem::exists(_account_config_file_path)) {
      root = json();
      root["admin_pass"] = md5("123456"); //默认密码是123456
      root["accounts"] = json::array();
      if(!_save())
        return false;
    }

    try {
      if(boost::filesystem::exists(_account_config_file_path)) {
        root = json();
        std::fstream  f(_account_config_file_path.c_str());
        if(!f) {
          log_error("load account config file {} failed!", _account_config_file_path);
          return false;
        }
        std::stringstream ss;
        ss << f.rdbuf();
        root = json::parse(ss.str());
        f.close();
      }
    } catch(...) {
      return false;
    }

    return true;
  }

  bool _save()
  {
    try {
      std::ofstream f(_account_config_file_path.c_str());
      if(!f) {
        log_error("write account config file {} failed!", _account_config_file_path);
        return false;
      }
      f << root.dump(2) << std::endl;
      f.close();
      return true;
    } catch(...) {}
    return false;
  }

  //根据名称查询账号,存在返回true，否则返回false
  bool queryAccount(const std::string& name, Account& re)
  {
    auto list = loadAllAccounts();
    for(auto a : list) {
      if(a.name == name) {
        re = a;
        return true;
      }
    }

    return false;
  }

  //当前登录的用户
  std::shared_ptr<Account> currentLoginedAccount()
  {
    return _current;
  }

  //登录
  //成功返回true,失败返回false
  bool login(const std::string& name, const std::string& pass)
  {
    auto list = loadAllAccounts();
    for(auto l : list) {
      if(l.name == name
         && l.pass_crypted == md5(pass)) {
        _current = std::make_shared<Account>(l);
        changeLastLoginTime(name, get_timestamp());
        return true;
      }
    }
    return false;
  }

  bool changeLastLoginTime(const std::string& name, int64_t timestamp)
  {
    _load();
    json r;
    r["admin_pass"] = root["admin_pass"];
    r["accounts"] = json::array();
    for(auto j : root["accounts"]) {
      auto account = Account::loadFromJson(j.dump());
      if(account.name == name) {
        account.last_login_timestamp = timestamp;
        r["accounts"].push_back(json::parse(account.dumpToJson()));
      } else {
        r["accounts"].push_back(j);
      }
    }
    root = r;
    return _save();
  }

  //登出
  //成功返回true,失败返回false
  bool logout()
  {
    _current = NULL;
    return true;
  }

  //加载所有账号
  std::list<Account> loadAllAccounts()
  {
    _load();

    std::list<Account> re;
    json ls = root["accounts"];
    for(auto& w : ls) {
      auto account = Account::loadFromJson(w.dump());
      re.push_back(account);
    }
    return re;
  }

  //删除账号
  //成功返回true,失败返回false
  bool deleteAccount(const std::string& name)
  {
    _load();
    json r;
    r["admin_pass"] = root["admin_pass"];
    r["accounts"] = json::array();
    for(auto j : root["accounts"]) {
      auto account = Account::loadFromJson(j.dump());
      if(account.name != name)
        r["accounts"].push_back(j);
    }
    root = r;
    return _save();
  }

  //添加账号
  //成功返回true,失败返回false
  //账号名已经存在返回false
  bool addAccount(const std::string& name, const std::string& pass)
  {
    Account a;
    if(queryAccount(name, a)) {
      return false;
    }

    a.name = name;
    a.pass_crypted = md5(pass);
    a.last_login_timestamp = 0;

    root["accounts"].push_back(json::parse(a.dumpToJson()));
    return _save();
  }

  //修改密码
  //成功返回true,失败返回false
  bool changeAccountPass(const std::string& name, const std::string& new_pass)
  {
    _load();
    json r;
    r["admin_pass"] = root["admin_pass"];
    r["accounts"] = json::array();
    for(auto j : root["accounts"]) {
      auto account = Account::loadFromJson(j.dump());
      if(account.name == name) {
        account.pass_crypted = md5(new_pass);
        r["accounts"].push_back(json::parse(account.dumpToJson()));
      } else {
        r["accounts"].push_back(j);
      }
    }
    root = r;
    return _save();
  }


  //验证管理员密码
  //成功返回true,失败返回false
  bool checkAdminPass(const std::string& pass)
  {
    _load();
    auto pss = root["admin_pass"].get<std::string>();
    return pss == md5(pass);
  }

  //修改管理员密码
  //成功返回true,失败返回false
  bool changeAdminPass(const std::string& old_pass,
                       const std::string& new_pass)
  {
    _load();
    auto pss = root["admin_pass"].get<std::string>();
    if(pss == md5(old_pass)) {
      root["admin_pass"] = md5(new_pass);
      return _save();
    }
    return false;
  }
};


//--------------------------------------------------------------------
AccountManager* AccountManager::get()
{
  static AccountManager* inst = new AccountManager();
  return inst;
}

AccountManager::AccountManager()
{
  _p = new AccountManagerPrivate();
}

AccountManager::~AccountManager()
{
  delete _p;
}

//当前登录的用户
std::shared_ptr<Account> AccountManager::currentLoginedAccount()
{
  return _p->currentLoginedAccount();
}

//登录
//成功返回true,失败返回false
bool AccountManager::login(const std::string& name, const std::string& pass)
{
  return _p->login(name, pass);
}

//登出
//成功返回true,失败返回false
bool AccountManager::logout()
{
  return _p->logout();
}

//加载所有账号
std::list<Account> AccountManager::loadAllAccounts()
{
  return _p->loadAllAccounts();
}

bool AccountManager::isAccountExist(const std::string& name)
{
  Account a;
  if(_p->queryAccount(name, a)) {
    return true;
  }
  return false;
}

//删除账号
//成功返回true,失败返回false
bool AccountManager::deleteAccount(const std::string& name)
{
  return _p->deleteAccount(name);
}

//添加账号
//成功返回true,失败返回false
bool AccountManager::addAccount(const std::string& name, const std::string& pass)
{
  return _p->addAccount(name, pass);
}

//修改密码
//成功返回true,失败返回false
bool AccountManager::changeAccountPass(const std::string& name, const std::string& new_pass)
{
  return _p->changeAccountPass(name, new_pass);
}


//验证管理员密码
//成功返回true,失败返回false
bool AccountManager::checkAdminPass(const std::string& pass)
{
  return _p->checkAdminPass(pass);
}

//修改管理员密码
//成功返回true,失败返回false
bool AccountManager::changeAdminPass(const std::string& old_pass,
                                     const std::string& new_pass)
{
  return _p->changeAdminPass(old_pass, new_pass);
}
