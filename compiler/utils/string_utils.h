#ifndef PICKC_UTILS_STRING_UTILS_H_
#define PICKC_UTILS_STRING_UTILS_H_

#include <string>
#include <vector>

namespace pickc
{
  bool startsWith(const std::string& str, const std::string& prefix);
  bool endsWith(const std::string& str, const std::string& suffix);
  std::vector<std::string> split(const std::string& str, const std::string& sep);
}

#endif // PICKC_UTILS_STRING_UTILS_H_