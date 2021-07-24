#ifndef PICKC_UTILS_VECTOR_UTILS_H_
#define PICKC_UTILS_VECTOR_UTILS_H_

#include <vector>
#include <cassert>
namespace pickc
{
  template<typename T>
  std::vector<T> operator+(const std::vector<T>& v1, const std::vector<T>& v2)
  {
    std::vector<T> result(v1.size() + v2.size());
    result.insert(result.end(), v1.begin(), v1.end());
    result.insert(result.end(), v2.begin(), v2.end());
    return result;
  }
  template<typename T>
  std::vector<T>& operator+=(std::vector<T>& v1, const std::vector<T>& v2)
  {
    v1.reserve(v1.size() + v2.size());
    v1.insert(v1.end(), v2.begin(), v2.end());
    return v1;
  }
  template<typename T>
  inline bool includes(const std::vector<T>& vec, const T& value)
  {
    return std::find(vec.begin(), vec.end(), value) != vec.end();
  }
  template<typename T>
  size_t indexOf(const std::vector<T>& vec, const T& value)
  {
    assert(std::find(vec.begin(), vec.end(), value) != vec.end());
    return std::distance(vec.begin(), std::find(vec.begin(), vec.end(), value));
  }
  template<typename T, typename F>
  auto map(const std::vector<T>& vec, F& fn)
  {
    std::vector<decltype(fn(vec.at(0)))> res;
    for(const auto& v : vec) {
      res.push_back(fn(v));
    }
    return res;
  }
}

#endif // PICKC_UTILS_VECTOR_UTILS_H_