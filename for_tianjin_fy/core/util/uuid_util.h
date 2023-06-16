#ifndef CORE_UTIL_UUID_UTIL_H_
#define CORE_UTIL_UUID_UTIL_H_

#include <string>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

/**
 * 用于产生唯一的id序列号
 */

inline std::string uuid()
{
  boost::uuids::random_generator  uuid_v4;
  return boost::uuids::to_string(uuid_v4());
}

#endif //CORE_UTIL_UUID_UTIL_H_
