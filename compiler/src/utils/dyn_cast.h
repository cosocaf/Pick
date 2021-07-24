#ifndef PICKC_UTILS_DYN_CAST_H_
#define PICKC_UTILS_DYN_CAST_H_

#include <cassert>
#include <memory>

namespace pickc
{
  template<typename To, typename From>
  inline To* dynCast(From* from)
  {
    assert(dynamic_cast<To*>(from));
    return dynamic_cast<To*>(from);
  }
  template<typename To, typename From>
  inline const To* dynCast(const From* from)
  {
    assert(dynamic_cast<const To*>(from));
    return dynamic_cast<const To*>(from);
  }
  template<typename To, typename From>
  inline std::shared_ptr<decltype(*(To()))> dynCast(std::shared_ptr<From> from) {
    assert(std::dynamic_pointer_cast<decltype(*(To()))>(from));
    return std::dynamic_pointer_cast<decltype(*(To()))>(from);
  }
}

#endif // PICKC_UTILS_DYN_CAST_H_