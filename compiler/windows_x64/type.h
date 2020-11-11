#ifndef PICKC_WINDOWS_X64_TYPE_H_
#define PICKC_WINDOWS_X64_TYPE_H_

#include <unordered_map>

#include "pcir/pcir_struct.h"

namespace pickc::windows::x64
{
  struct Type
  {
    pcir::Types type;
    union {

    };
    size_t getSize() const;
    Type();
    Type(pcir::Types langDefType);
  };
  class TypeTable
  {
    std::unordered_map<pcir::TypeSection*, Type> types;
  public:
    TypeTable();
    const Type& operator[](pcir::TypeSection* pcirType);
  };
}

#endif // PICKC_WINDOWS_X64_TYPE_H_