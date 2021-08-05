/**
 * @file dyn_cast.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief dynamic_castのラッパー。
 * 通常のdynamic_castとshared_ptr用のdynamic_pointer_castを意識せずに使えるようにオーバーロードしている。
 * @version 0.1
 * @date 2021-07-31
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_UTILS_DYN_CAST_H_
#define PICKC_UTILS_DYN_CAST_H_

#include <cassert>
#include <memory>

namespace pickc
{
  /**
   * @brief From型ポインタからTo型ポインタへキャストする。
   * キャストできない場合はassertで落ちるようになっている。
   * そのため、事前にinstanceofを使用して、キャストできるかチェックする。
   * @tparam To 
   * @tparam From 
   * @param from 
   * @return To* 
   */
  template<typename To, typename From>
  inline To* dynCast(From* from) {
    assert(dynamic_cast<To*>(from));
    return dynamic_cast<To*>(from);
  }
  template<typename To, typename From>
  inline const To* dynCast(const From* from) {
    assert(dynamic_cast<const To*>(from));
    return dynamic_cast<const To*>(from);
  }
  template<typename To, typename From>
  inline To dynCast(std::shared_ptr<From> from) {
    assert(std::dynamic_pointer_cast<To::element_type>(from));
    return std::dynamic_pointer_cast<To::element_type>(from);
  }
}

#endif // PICKC_UTILS_DYN_CAST_H_