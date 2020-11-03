#include "string_utils.h"

namespace pickc
{
  bool startsWith(const std::string& str, const std::string& prefix)
  {
    if(str.size() < prefix.size()) return false;
    return std::equal(std::begin(prefix), std::end(prefix), std::begin(str));
  }
  bool endsWith(const std::string& str, const std::string& suffix)
  {
    if(str.size() < suffix.size()) return false;
    return std::equal(std::rbegin(suffix), std::rend(suffix), std::rbegin(str));
  }
  std::vector<std::string> split(const std::string& str, const std::string& sep)
  {
    if(str.size() < sep.size()) return { str };
    std::vector<std::string> result;
    std::string buf;
    for(size_t i = 0, l = str.size() - sep.size(); i < l; ++i) {
      bool eq = true;
      for(size_t j = 0, jl = sep.size(); j < jl; ++j) {
        if(str[i + j] != sep[j]) {
          eq = false;
          break;
        }
      }
      if(eq && !buf.empty()) {
        result.push_back(buf);
        buf.clear();
      }
      else {
        buf += str[i];
      }
    }
    if(!buf.empty()) result.push_back(buf);
    return result;
  }
}