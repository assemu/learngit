#ifndef CORE_COMMON_ACCOUNT_H_
#define CORE_COMMON_ACCOUNT_H_

#include <string>
#include <core/util/json_util.h>
#include <core/util/log.h>

class Account
{
public:
  std::string name;
  std::string pass_crypted;
  int64_t last_login_timestamp;

  Account(const Account& other)
  {
    name = other.name;
    pass_crypted = other.pass_crypted;
    last_login_timestamp = other.last_login_timestamp;
  }

  Account()
  {
    name = "";
    pass_crypted = "";
    last_login_timestamp = 0;
  }

  //序列化和反序列化
  std::string dumpToJson() const
  {
    std::string re;
    try {
      nlohmann::json j;
      j["name"] = name;
      j["pass"] = pass_crypted;
      j["last_login_timestamp"] = last_login_timestamp;
      re = j.dump(2);
    } catch(...) {
      log_error("dump Account to json error!");
    }
    return re;
  }

  static Account loadFromJson(const std::string& json_str)
  {
    Account re;
    try {
      auto j = nlohmann::json::parse(json_str);
      re.name = j["name"].get<std::string>();
      re.pass_crypted = j["pass"].get<std::string>();
      re.last_login_timestamp = j["last_login_timestamp"].get<int64_t>();
    } catch(...) {
      log_error("load Account from json error!");
    }
    return re;
  }
};

#endif //CORE_COMMON_ACCOUNT_H_
