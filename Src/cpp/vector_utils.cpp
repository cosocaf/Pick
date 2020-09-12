#include "vector_utils.h"

namespace pick
{
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint8_t val)
    {
        vec.push_back(val);
        return vec;
    }
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint16_t val)
    {
        vec.push_back(val & 0xFF);
        vec.push_back((val >> 8) & 0xFF);
        return vec;
    }
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint32_t val)
    {
        vec.push_back(val & 0xFF);
        vec.push_back((val >> 8) & 0xFF);
        vec.push_back((val >> 16) & 0xFF);
        vec.push_back((val >> 24) & 0xFF);
        return vec;
    }
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint64_t val)
    {
        vec.push_back(val & 0xFF);
        vec.push_back((val >> 8) & 0xFF);
        vec.push_back((val >> 16) & 0xFF);
        vec.push_back((val >> 24) & 0xFF);
        vec.push_back((val >> 32) & 0xFF);
        vec.push_back((val >> 40) & 0xFF);
        vec.push_back((val >> 48) & 0xFF);
        vec.push_back((val >> 56) & 0xFF);
        return vec;
    }
    std::vector<uint8_t> toList(uint8_t u8)
    {
        return { u8 };
    }
    std::vector<uint8_t> toList(uint16_t u16)
    {
        return {
            static_cast<uint8_t>(u16 & 0xFF),
            static_cast<uint8_t>((u16 >> 8) & 0xFF)
        };
    }
    std::vector<uint8_t> toList(uint32_t u32)
    {
        return {
            static_cast<uint8_t>(u32 & 0xFF),
            static_cast<uint8_t>((u32 >> 8) & 0xFF),
            static_cast<uint8_t>((u32 >> 16) & 0xFF),
            static_cast<uint8_t>((u32 >> 24) & 0xFF)
        };
    }
    std::vector<uint8_t> toList(uint64_t u64)
    {
        return {
            static_cast<uint8_t>(u64 & 0xFF),
            static_cast<uint8_t>((u64 >> 8) & 0xFF),
            static_cast<uint8_t>((u64 >> 16) & 0xFF),
            static_cast<uint8_t>((u64 >> 24) & 0xFF),
            static_cast<uint8_t>((u64 >> 32) & 0xFF),
            static_cast<uint8_t>((u64 >> 40) & 0xFF),
            static_cast<uint8_t>((u64 >> 48) & 0xFF),
            static_cast<uint8_t>((u64 >> 56) & 0xFF)
        };
    }
    std::vector<std::string> charVecToStrVec(const std::vector<const char*> vec)
    {
        std::vector<std::string> res(vec.size());
        for (size_t i = 0, l = vec.size(); i < l; ++i) {
            res[i] = vec[i];
        }
        return res;
    }
    std::vector<std::string> charToStrVec(const char* vec)
    {
        assert(vec != nullptr);
        std::vector<std::string> res;
        res += vec;
        return res;
    }
}