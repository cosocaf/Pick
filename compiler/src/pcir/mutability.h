/**
 * @file mutability.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-05
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_MUTABILITY_H_
#define PICKC_PCIR_MUTABILITY_H_

#include <cstdint>

namespace pickc::pcir {
  enum struct Mutability : uint16_t {
    Mutable = 0x01,
    Immutable = 0x02,
  };
}

#endif // PICKC_PCIR_MUTABILITY_H_