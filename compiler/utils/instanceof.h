#ifndef PICKC_UTILS_INSTANCEOF_H_
#define PICKC_UTILS_INSTANCEOF_H_

namespace pickc
{
  template <typename Of, typename What>
  inline bool instanceof(const What w)
  {
    return dynamic_cast<const Of*>(w) != nullptr;
  }
}

#endif // PICKC_UTILS_INSTANCEOF_H_