/**
 * @file routine_converter.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-27
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_ROUTINE_CONVERTER_H_
#define PICKC_LINKER_ROUTINE_CONVERTER_H_

#include "routine.h"
#include "operator.h"

#include "bundler/bundle.h"

namespace pickc::linker {
  class RoutineConverter {
    Routine routine;
    bundler::WeakBundle bundle;
    bundler::InternalFunction function;
  public:
    RoutineConverter(const bundler::WeakBundle& bundle, const bundler::InternalFunction& function);
    Routine convert();
  private:
    std::vector<Operator> analyzeBytecode(const BinaryVec& bytecode);
  };
}

#endif // PICKC_LINKER_ROUTINE_CONVERTER_H_