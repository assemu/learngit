#ifndef UTIL_STR_UTIL_H_
#define UTIL_STR_UTIL_H_

#include <string>
#include <sstream>
#include <fmt/format.h>
#include <boost/locale/encoding.hpp>
#include <boost/locale.hpp>
#include <boost/lexical_cast.hpp>

#ifdef _WIN32_WINNT
#include <windows.h>
#endif

inline void str_trim(std::string& str)
{
  std::string blanks("\f\v\r\t\n ");
  str.erase(0, str.find_first_not_of(blanks));
  str.erase(str.find_last_not_of(blanks) + 1);
}

inline std::string str_system_to_utf8(const std::string& str)
{
#ifdef _WIN32_WINNT
  //windows stl抛出的异常编码不是utf8
  static std::string codepage;
  if(codepage.empty()) { // 获得系统当前的代码页
    CPINFOEX  cpinfo;
    GetCPInfoEx(CP_ACP, 0, &cpinfo);
    cpinfo.CodePageName;
    codepage = "CP" + boost::lexical_cast<std::string>(cpinfo.CodePage);
  }
  return boost::locale::conv::between(str, "UTF-8", codepage.c_str());
#else
  return str;
#endif
}

inline std::string str_utf8_to_system(const std::string& str)
{
#ifdef _WIN32_WINNT
  //windows stl抛出的异常编码不是utf8
  static std::string codepage;
  if(codepage.empty()) { // 获得系统当前的代码页
    CPINFOEX  cpinfo;
    GetCPInfoEx(CP_ACP, 0, &cpinfo);
    cpinfo.CodePageName;
    codepage = "CP" + boost::lexical_cast<std::string>(cpinfo.CodePage);
  }
  return boost::locale::conv::between(str, codepage.c_str(), "UTF-8");
#else
  return str;
#endif
}

inline std::vector<std::string> str_split_string(std::string srcStr, std::string delimStr, bool repeatedCharIgnored = true)
{
  std::vector<std::string> resultStringVector;
  std::replace_if(srcStr.begin(), srcStr.end(),
  [&](const char& c) {
    if(delimStr.find(c) != std::string::npos) {
      return true;
    } else {
      return false;
    }
  }/*pred*/,
  delimStr.at(0));

  size_t pos = srcStr.find(delimStr.at(0));
  std::string addedString = "";
  while(pos != std::string::npos) {
    addedString = srcStr.substr(0, pos);
    if(!addedString.empty() || !repeatedCharIgnored) {
      resultStringVector.push_back(addedString);
    }
    srcStr.erase(srcStr.begin(), srcStr.begin() + pos + 1);
    pos = srcStr.find(delimStr.at(0));
  }

  addedString = srcStr;
  if(!addedString.empty() || !repeatedCharIgnored) {
    resultStringVector.push_back(addedString);
  }

  return resultStringVector;
}

/**
 * 字符串格式化、编码等相关的工具函数
 */
template <typename S, typename... Args>
inline std::string format(const S& format_str, const Args& ... args)
{
  fmt::memory_buffer buf;
  fmt::format_to(buf, format_str, args...);
  std::stringstream ss;
  ss << buf.data();
  return ss.str();
}

#endif //UTIL_STR_UTIL_H_
