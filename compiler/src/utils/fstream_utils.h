/**
 * @file fstream_utils.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_UTILS_FSTREAM_UTILS_H_
#define PICKC_UTILS_FSTREAM_UTILS_H_

#include <fstream>
#include <cassert>

namespace pickc {
  template<typename T>
  std::ifstream& operator>>(std::ifstream& stream, T& value) {
    assert(stream.is_open() && stream.good());
    stream.read((char*)&value, sizeof(value));
    assert(stream.good());
    return stream;
  }
}

#endif // PICKC_UTILS_FSTREAM_UTILS_H_