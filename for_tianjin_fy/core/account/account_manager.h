#ifndef CORE_COMMON_ACCOUNT_MANAGER_H_
#define CORE_COMMON_ACCOUNT_MANAGER_H_

#include <list>
#include <memory>
#include <string>
#include <vector>
#include "account.h"

//本地配置文件账号管理
class AccountManagerPrivate;
class AccountManager
{
public:
  static AccountManager* get();

  //当前登录的用户
  std::shared_ptr<Account> currentLoginedAccount();

  //登录
  //成功返回true,失败返回false
  bool login(const std::string& name, const std::string& pass);

  //登出
  //成功返回true,失败返回false
  bool logout();

  //加载所有账号
  std::list<Account> loadAllAccounts();

  //账号是否存在
  bool isAccountExist(const std::string& name);

  //删除账号
  //成功返回true,失败返回false
  bool deleteAccount(const std::string& name);

  //添加账号
  //成功返回true,失败返回false
  //账号名已经存在返回false
  bool addAccount(const std::string& name, const std::string& pass);

  //修改密码
  //成功返回true,失败返回false
  bool changeAccountPass(const std::string& name, const std::string& new_pass);


  //验证管理员密码
  //成功返回true,失败返回false
  bool checkAdminPass(const std::string& pass);

  //修改管理员密码
  //成功返回true,失败返回false
  bool changeAdminPass(const std::string& old_pass,
                       const std::string& new_pass);

private:
  AccountManager();
  ~AccountManager();
  AccountManagerPrivate* _p; //代理，隐藏成员变量，隐藏实现机制
};

#endif //CORE_COMMON_ACCOUNT_MANAGER_H_
