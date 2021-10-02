/**
 * @file bundler.cpp
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#include "bundler.h"

#include <cassert>

#include "pcir_decoder.h"

namespace pickc::bundler {
  Bundler::Bundler(const std::vector<std::string>& filenames) : filenames(filenames) {}
  std::optional<Bundle> Bundler::bundle() {
    auto res = _Bundle::create();
    for(auto& filename : filenames) {
      PCIRDecoder pcirDecoder(filename);
      auto decode = pcirDecoder.decode();
      if(decode) {
        res->insert(std::move(decode.value()));
      }
      else {
        // TODO: return error.
        assert(false);
      }
    }

    return res;
  }
}