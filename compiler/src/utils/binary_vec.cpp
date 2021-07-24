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
  float getF32(const BinaryVec& vec, size_t& i)
  {
    return *reinterpret_cast<float*>(&subVec(vec, i, 4)[0]);
  }
  double getF64(const BinaryVec& vec, size_t& i)
  {
    return *reinterpret_cast<double*>(&subVec(vec, i, 8)[0]);
  }
  BinaryVec subVec(const BinaryVec& vec, size_t& i, size_t len)
  {
    BinaryVec result;
    result.reserve(len);
    for(size_t count = 0; count < len; ++count, ++i) {
      result.push_back(vec[i]);
    }
    return result;
  }
  BinaryVec toVec(int8_t i8) {
    return BinaryVec{ static_cast<uint8_t>((i8 >> 0) & 0xff) };
  }
  BinaryVec toVec(int16_t i16) {
    return BinaryVec{
      static_cast<uint8_t>((i16 >> 0) & 0xff),
      static_cast<uint8_t>((i16 >> 8) & 0xff)
    };
  }
  BinaryVec toVec(int32_t i32) {
    return BinaryVec{
      static_cast<uint8_t>((i32 >> 0) & 0xff),
      static_cast<uint8_t>((i32 >> 8) & 0xff),
      static_cast<uint8_t>((i32 >> 16) & 0xff),
      static_cast<uint8_t>((i32 >> 24) & 0xff)
    };
  }
  BinaryVec toVec(int64_t i64) {
    return BinaryVec{
      static_cast<uint8_t>((i64 >> 0) & 0xff),
      static_cast<uint8_t>((i64 >> 8) & 0xff),
      static_cast<uint8_t>((i64 >> 16) & 0xff),
      static_cast<uint8_t>((i64 >> 24) & 0xff),
      static_cast<uint8_t>((i64 >> 32) & 0xff),
      static_cast<uint8_t>((i64 >> 40) & 0xff),
      static_cast<uint8_t>((i64 >> 48) & 0xff),
      static_cast<uint8_t>((i64 >> 56) & 0xff)
    };
  }
  BinaryVec toVec(uint8_t u8) {
    return BinaryVec{ u8 };
  }
  BinaryVec toVec(uint16_t u16) {
    return BinaryVec{
      static_cast<uint8_t>((u16 >> 0) & 0xff),
      static_cast<uint8_t>((u16 >> 8) & 0xff)
    };
  }
  BinaryVec toVec(uint32_t u32) {
    return BinaryVec{
      static_cast<uint8_t>((u32 >> 0) & 0xff),
      static_cast<uint8_t>((u32 >> 8) & 0xff),
      static_cast<uint8_t>((u32 >> 16) & 0xff),
      static_cast<uint8_t>((u32 >> 24) & 0xff)
    };
  }
  BinaryVec toVec(uint64_t u64) {
    return BinaryVec{
      static_cast<uint8_t>((u64 >> 0) & 0xff),
      static_cast<uint8_t>((u64 >> 8) & 0xff),
      static_cast<uint8_t>((u64 >> 16) & 0xff),
      static_cast<uint8_t>((u64 >> 24) & 0xff),
      static_cast<uint8_t>((u64 >> 32) & 0xff),
      static_cast<uint8_t>((u64 >> 40) & 0xff),
      static_cast<uint8_t>((u64 >> 48) & 0xff),
      static_cast<uint8_t>((u64 >> 56) & 0xff)
    };
  }
}