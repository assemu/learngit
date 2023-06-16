#ifndef UTIL_LOG_H_
#define UTIL_LOG_H_

#include <spdlog/spdlog.h>

/*
 * 用于记录日志到控制台或者文件
 */

//#define DEFAULT_LOG_PATTERN "[%m/%d %T.%F] [%^%=8l%$] [%6P/%-6t] [%@#%!] %v"
#define DEFAULT_LOG_PATTERN "[%m/%d %T] [%^%=8l%$] %v"

std::shared_ptr<spdlog::logger> get_logger();

template<typename... Args>
inline void log_trace(const char* fmt, const Args& ... args)
{
  if(get_logger())
    get_logger()->trace(fmt, args...);
}

template<typename... Args>
inline void log_debug(const char* fmt, const Args& ... args)
{
  if(get_logger())
    get_logger()->debug(fmt, args...);
}

template<typename... Args>
inline void log_info(const char* fmt, const Args& ... args)
{
  if(get_logger())
    get_logger()->info(fmt, args...);
}

template<typename... Args>
inline void log_warn(const char* fmt, const Args& ... args)
{
  if(get_logger())
    get_logger()->warn(fmt, args...);
}

template<typename... Args>
inline void log_error(const char* fmt, const Args& ... args)
{
  if(get_logger())
    get_logger()->error(fmt, args...);
}

template<typename... Args>
inline void log_critical(const char* fmt, const Args& ... args)
{
  if(get_logger())
    get_logger()->critical(fmt, args...);
}

void setup_log(const std::string& log_file_path, const std::string& level);

void setup_log_console(const std::string& level);

#endif //UTIL_LOG_H_
