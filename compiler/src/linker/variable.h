/**
 * @file variable.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LINKER_VARIABLE_H_
#define PICKC_LINKER_VARIABLE_H_

#include <memory>

#include "bundler/bundle.h"

namespace pickc::linker {
  struct _Variable;
  
  using Variable = std::shared_ptr<_Variable>;
  using WeakVariable = std::weak_ptr<_Variable>;

  struct _Variable {
    bundler::TypeInfo type;
    // この変数が使用するメモリ領域の大きさ
    int32_t memoryArea;
    // この変数が配置されるメモリ領域のオフセットアドレス
    int32_t memoryOffset;
  };
}

#endif // PICKC_LINKER_VARIABLE_H_