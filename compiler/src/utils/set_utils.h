#ifndef PICKC_UTILS_SET_UTILS_H_
#define PICKC_UTILS_SET_UTILS_H_

#include <set>
#include <unordered_set>

namespace pickc
{
  template<typename T>
  inline size_t indexOf(const std::set<T>& set, const T& value)
  {
    assert(set.find(value) != set.end());
    return std::distance(set.begin(), set.find(value));
  }
  template<typename T>
  inline bool exists(const std::set<T>& set, const T& value)
  {
    return set.find(value) != set.end();
  }
  template<typename T>
  inline bool exists(const std::unordered_set<T>& set, const T& value)
  {
    return set.find(value) != set.end();
  }
}

#endif // PICKC_UTILS_SET_UTILS_H_