/**
 * @file linker.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-06
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_LINKER_H_
#define PICKC_LINKER_LINKER_H_

#include <map>
#include <array>
#include <vector>

#include "routine.h"

#include "bundler/bundle.h"

namespace pickc::linker {
  class Linker {
    bundler::Bundle bundle;
    std::map<std::array<uint8_t, 16>, std::vector<Routine>> routines;
  public:
    Linker(const bundler::Bundle& bundle);
    bool link();
    bool writeToFile(const std::string& filename);
  };
}

#endif // PICKC_LINKER_LINKER_H_