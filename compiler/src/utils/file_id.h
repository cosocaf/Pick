/**
 * @file file_id.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-07-28
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_UTILS_FILE_ID_H_
#define PICKC_UTILS_FILE_ID_H_

#include <array>

namespace pickc {
  /**
   * @brief PCIRのfileID用にランダムなIDを生成する。
   * デフォルトの実装ではUUID v4を返すように設計されている。
   * 
   * @return std::array<uint8_t, 16> 生成されたID
   */
  std::array<uint8_t, 16> genFileID();
}

#endif // PICKC_UTILS_FILE_ID_H_