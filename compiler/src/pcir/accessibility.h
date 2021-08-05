/**
 * @file accessibility.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-05
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_PCIR_ACCESSIBILITY_H_
#define PICKC_PCIR_ACCESSIBILITY_H_

#include <cstdint>

namespace pickc::pcir {
  enum struct Accessibility : uint16_t {
    Public = 0x01,
    Private = 0x02,
  };
}

#endif // PICKC_PCIR_ACCESSIBILITY_H_