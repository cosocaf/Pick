#pragma once

#include <vector>
#include <string>
#include <cassert>

namespace pick
{
    template<typename T>
    std::vector<T>& operator+=(std::vector<T>& a, const std::vector<T>& b)
    {
        a.reserve(a.size() + b.size());
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }
    template<typename T>
    std::vector<T>& operator+=(std::vector<T>& a, std::vector<T>&& b)
    {
        a.reserve(a.size() + b.size());
        a.insert(a.end(), std::make_move_iterator(b.begin()), std::make_move_iterator(b.end()));
        return a;
    }
    template<typename T>
    std::vector<T>& operator+=(std::vector<T>& a, std::vector<T>& b)
    {
        a.reserve(a.size() + b.size());
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }
    template<typename A, typename B>
    std::vector<A>& operator+=(std::vector<A>& a, const B& b)
    {
        a.push_back(b);
        return a;
    }
    template<typename A, typename B>
    std::vector<A>& operator+=(std::vector<A>& a, B&& b)
    {
        a.emplace_back(std::move(b));
        return a;
    }
    /*template<typename T>
    std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
    {
        std::vector<T> res(a.size() + b.size());
        res.insert(res.end(), a.begin(), a.end());
        res.insert(res.end(), b.begin(), b.end());
        return res;
    }
    template<typename T>
    std::vector<T> operator+(std::vector<T>&& a, std::vector<T>&& b)
    {
        std::vector<T> res(a.size() + b.size());
        res.insert(res.end(), std::make_move_iterator(a.begin()), std::make_move_iterator(a.end()));
        res.insert(res.end(), std::make_move_iterator(b.begin()), std::make_move_iterator(b.end()));
        return res;
    }*/

    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint8_t val);
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint16_t val);
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint32_t val);
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, uint64_t val);

    template<typename T>
    std::vector<uint8_t>& operator<<(std::vector<uint8_t>& vec, const T& val)
    {
        for (int i = 0; i < sizeof(T); ++i) {
            vec.push_back(((char*)&val)[i]);
        }
        return vec;
    }

    std::vector<uint8_t> toList(uint8_t u8);
    std::vector<uint8_t> toList(uint16_t u16);
    std::vector<uint8_t> toList(uint32_t u32);
    std::vector<uint8_t> toList(uint64_t u64);

    std::vector<std::string> charVecToStrVec(const std::vector<const char*> vec);
    std::vector<std::string> charToStrVec(const char* vec);

    inline void pushError(std::vector<std::string>& error, const std::string& message)
    {
#ifdef _DEBUG
        assert(false);
#else
        error.push_back(message);
#endif
    }
}