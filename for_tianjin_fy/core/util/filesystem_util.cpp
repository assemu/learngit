#include <boost/asio.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "filesystem_util.h"
#include "str_util.h"

bool copy_directory(const std::string& src, const std::string& dest)
{
  if(!file_exist(src))
    return false;

  auto _src = str_utf8_to_system(src);
  auto _dest = str_utf8_to_system(dest);

  boost::filesystem::recursive_directory_iterator end;
  boost::system::error_code ec;
  for(boost::filesystem::recursive_directory_iterator pos(_src); pos != end; ++pos) {
    if(boost::filesystem::is_directory(*pos))
      continue;
    std::string strAppPath = boost::filesystem::path(*pos).string();
    std::string strRestorePath;
    boost::replace_first_copy(std::back_inserter(strRestorePath), strAppPath, _src, _dest);
    if(!boost::filesystem::exists(boost::filesystem::path(strRestorePath).parent_path())) {
      boost::filesystem::create_directories(boost::filesystem::path(strRestorePath).parent_path(), ec);
    }
    boost::filesystem::copy_file(strAppPath, strRestorePath, boost::filesystem::copy_option::overwrite_if_exists, ec);
  }
  if(ec) {
    return false;
  }
  return true;
}

bool file_exist(const std::string& file_path)
{
  return boost::filesystem::exists(str_utf8_to_system(file_path));
}

bool create_path(const std::string& path)
{
  try {
    boost::filesystem::create_directories(str_utf8_to_system(path));
    return true;
  } catch(...) {}
  return false;
}

bool file_remove(const std::string& file_path)
{
  try {
    boost::filesystem::remove(str_utf8_to_system(file_path));
    return true;
  } catch(...) {}
  return false;
}

std::string bin_dir()
{
  return boost::filesystem::initial_path<boost::filesystem::path>().string();
}
