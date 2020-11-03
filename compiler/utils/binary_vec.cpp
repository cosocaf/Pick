#include "binary_vec.h"

namespace pickc
{
  template<>
  BinaryVec& operator<<(BinaryVec& vec, const BinaryVec& value)
  {
    vec.reserve(vec.size() + value.size());
    vec.insert(vec.end(), value.begin(), value.end());
    return vec;
  }
  template<>
  BinaryVec& operator<<(BinaryVec& vec, const std::string& value)
  {
    vec.reserve(vec.size() + value.size());
    vec.insert(vec.end(), value.begin(), value.end());
    return vec;
  }
  uint8_t get8(const BinaryVec& vec, size_t& i)
  {
    return vec[++i];
  }
  uint16_t get16(const BinaryVec& vec, size_t& i)
  {
    uint16_t u16 = 0;
    u16 |= (vec[++i] << 0);
    u16 |= (vec[++i] << 8);
    return u16;
  }
  uint32_t get32(const BinaryVec& vec, size_t& i)
  {
    uint32_t u32 = 0;
    u32 |= (vec[++i] << 0);
    u32 |= (vec[++i] << 8);
    u32 |= (vec[++i] << 16);
    u32 |= (vec[++i] << 24);
    return u32;
  }
  uint64_t get64(const BinaryVec& vec, size_t& i)
  {
    uint64_t u64 = 0;
    u64 |= (static_cast<uint64_t>(vec[++i]) << 0);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 8);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 16);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 24);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 32);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 40);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 48);
    u64 |= (static_cast<uint64_t>(vec[++i]) << 56);
    return u64;
  }
}