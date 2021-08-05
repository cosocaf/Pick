/**
 * @file file_id.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "file_id.h"

#include <random>

namespace pickc {
  std::array<uint8_t, 16> genFileID() {
    std::array<uint8_t, 16> id;
    std::mt19937 gen(std::rand());
    for(int n = 0; n < 4; ++n) {
      auto rnd = gen();
      for(int m = 0; m < 4; ++m) {
        id[4 * n + m] = ((rnd & (0xFF << (8 * m))) >> (8 * m)) & 0xFF;
      }
    }
    id[6] = (id[6] & 0x0F) | 0x40;
    id[8] = (id[8] & 0x3F) | 0x80;
    return id;
  }
}