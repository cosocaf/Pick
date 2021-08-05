/**
 * @file accessibility.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief アクセシビリティ
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
  /**
   * @brief アクセシビリティを表す列挙。
   * 
   */
  enum struct Accessibility : uint16_t {
    /**
     * @brief Public。どの場所からもアクセス可能であることを表す。
     * 
     */
    Public = 0x01,
    /**
     * @brief Private。これが定義された場所からのみアクセス可能であることを表す。
     * 
     */
    Private = 0x02,
  };
}

#endif // PICKC_PCIR_ACCESSIBILITY_H_