#include "log.h"
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <sstream>

static std::shared_ptr<spdlog::logger> logger = NULL;

void setup_log(const std::string& log_file_path, const std::string& level)
{
  if(!logger) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file_path, 0, 0);
    spdlog::sinks_init_list log_sinks = {console_sink, daily_sink};
    logger = std::make_shared<spdlog::logger>("app_log", log_sinks);
    logger->set_pattern(DEFAULT_LOG_PATTERN);
    logger->set_level(spdlog::level::from_str(level));
    spdlog::register_logger(logger);
  } else {
    log_warn("Setup log for more than once");
  }
}

void setup_log_console(const std::string& level)
{
  if(!logger) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    spdlog::sinks_init_list log_sinks = {console_sink};
    logger = std::make_shared<spdlog::logger>("app_log", log_sinks);
    logger->set_pattern(DEFAULT_LOG_PATTERN);
    logger->set_level(spdlog::level::from_str(level));
    spdlog::register_logger(logger);
  } else {
    log_warn("Setup log for more than once");
  }
}

std::shared_ptr<spdlog::logger> get_logger()
{
  return logger;
}
