#ifndef UTIL_HASH_UTIL_H_
#define UTIL_HASH_UTIL_H_

#include <string>
#include <boost/algorithm/hex.hpp>
#include <boost/uuid/detail/md5.hpp>
#include "private/MurmurHash3.h"

#define __UTIL_HASH_SEED 42

inline uint32_t hash_32(const unsigned char* key, int32_t length)
{
  uint32_t h;
  MurmurHash3_x86_32(key, length, __UTIL_HASH_SEED, &h);
  return h;
}

inline uint32_t hash_str_32(const std::string& key)
{
  return hash_32(reinterpret_cast<const unsigned char*>(key.c_str()), key.length());
}

inline std::string md5(const std::string& str)
{
  boost::uuids::detail::md5 boost_md5;
  boost_md5.process_bytes(str.data(), str.size());
  boost::uuids::detail::md5::digest_type digest;
  boost_md5.get_digest(digest);
  const auto char_digest = reinterpret_cast<const char*>(&digest);
  std::string str_md5;
  boost::algorithm::hex(char_digest,
                        char_digest + sizeof(boost::uuids::detail::md5::digest_type),
                        std::back_inserter(str_md5));
  return str_md5;
}

#endif //UTIL_HASH_UTIL_H_
