/**
 * @file bundler.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief PCIRのバンドラ。リンカの前処理。
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_BUNDLER_BUNDLER_H_
#define PICKC_BUNDLER_BUNDLER_H_

#include <vector>
#include <string>
#include <map>
#include <optional>

#include "bundle.h"
#include "pcir_decoder.h"

namespace pickc::bundler {
  class Bundler {
    std::vector<std::string> filenames;
  public:
    Bundler(const std::vector<std::string>& filenames);
    std::optional<Bundle> bundle();
  };
}

#endif // PICKC_BUNDLER_BUNDLER_H_