#ifndef PICKC_UTILS_BINARY_VEC_H_
#define PICKC_UTILS_BINARY_VEC_H_

#include <vector>
#include <string>

namespace pickc
{
  using BinaryVec = std::vector<uint8_t>;
  template<typename T>
  BinaryVec& operator<<(BinaryVec& vec, const T& value)
  {
    vec.reserve(vec.size() + sizeof(T));
    for(int i = 0; i < sizeof(T); ++i) {
      vec.push_back(*(((uint8_t*)&value) + i));
    }
    return vec;
  }
  template<>
  BinaryVec& operator<<(BinaryVec& vec, const BinaryVec& value);
  template<>
  BinaryVec& operator<<(BinaryVec& vec, const std::string& value);
  template<size_t N>
  BinaryVec& operator<<(BinaryVec& vec, const char (&value)[N])
  {
    vec.reserve(vec.size() + N - 1);
    vec.insert(vec.end(), std::begin(value), std::end(value) - 1);
    return vec;
  }
  uint8_t get8(const BinaryVec& vec, size_t& i);
  uint16_t get16(const BinaryVec& vec, size_t& i);
  uint32_t get32(const BinaryVec& vec, size_t& i);
  uint64_t get64(const BinaryVec& vec, size_t& i);
  float getF32(const BinaryVec& vec, size_t& i);
  double getF64(const BinaryVec& vec, size_t& i);
  BinaryVec subVec(const BinaryVec& vec, size_t& i, size_t len);
}

#endif // PICKC_UTILS_BINARY_VEC_H_