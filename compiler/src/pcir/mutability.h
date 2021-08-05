/**
 * @file mutability.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief ミュータビリティ
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
  /**
   * @brief ミュータビリティ。
   * 
   */
  enum struct Mutability : uint16_t {
    /**
     * @brief 変更可能であることを表す。
     * 
     */
    Mutable = 0x01,
    /**
     * @brief 変更不可能であることを表す。
     * 
     */
    Immutable = 0x02,
  };
}

#endif // PICKC_PCIR_MUTABILITY_H_