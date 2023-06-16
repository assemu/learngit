#ifndef UTIL_FILESYSTEM_UTIL_H_
#define UTIL_FILESYSTEM_UTIL_H_

#include <string>

bool file_exist(const std::string& file_path);
bool create_path(const std::string& path);
bool file_remove(const std::string& file_path);
bool copy_directory(const std::string& src, const std::string& dest);
std::string bin_dir();

#endif //UTIL_FILESYSTEM_UTIL_H_
