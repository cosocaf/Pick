#ifndef PICKC_UTILS_STRING_UTILS_H_
#define PICKC_UTILS_STRING_UTILS_H_

#include <string>
#include <vector>

namespace pickc
{
  bool startsWith(const std::string& str, const std::string& prefix);
  bool endsWith(const std::string& str, const std::string& suffix);
  std::vector<std::string> split(const std::string& str, const std::string& sep);
  std::string join(const std::vector<std::string>& vec, const std::string& delim = "");
  std::string textf(const char* const format, ...);
}

#endif // PICKC_UTILS_STRING_UTILS_H_