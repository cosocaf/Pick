/**
 * @file opecode.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */

namespace pickc {
  enum struct Opecode : uint8_t {
    ADD = 0x00,
    Imm8 = 0x80,
    Imm16 = 0x81,
    Imm32 = 0x82,
    Imm64 = 0x83,
    LoadFn = 0x90,
  };
}