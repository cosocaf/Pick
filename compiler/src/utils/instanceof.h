#ifndef PICKC_UTILS_INSTANCEOF_H_
#define PICKC_UTILS_INSTANCEOF_H_

namespace pickc
{
  template<typename Of, typename What>
  inline bool instanceof(const What w) {
    return dynamic_cast<Of*>(w) != nullptr;
  }
  template<typename Of, typename What>
  inline bool instanceof(std::shared_ptr<What> w) {
    return std::dynamic_pointer_cast<Of::element_type>(w) != nullptr;
  }
}

#endif // PICKC_UTILS_INSTANCEOF_H_