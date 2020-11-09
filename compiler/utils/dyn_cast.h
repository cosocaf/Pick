#ifndef PICKC_UTILS_DYN_CAST_H_
#define PICKC_UTILS_DYN_CAST_H_

#include <cassert>

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
}

#endif // PICKC_UTILS_DYN_CAST_H_