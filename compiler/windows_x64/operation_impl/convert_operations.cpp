#include "routine.h"

namespace pickc::windows::x64
{
  CWDOperation::CWDOperation() : Operation(OperationSize::QWord) {}
  CDQOperation::CDQOperation() : Operation(OperationSize::QWord) {}
  CQOOperation::CQOOperation() : Operation(OperationSize::QWord) {}
  BinaryVec CWDOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return { 0x66, 0x99 };
  }
  BinaryVec CDQOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return { 0x99 };
  }
  BinaryVec CQOOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return { 0x48, 0x99 };
  }
}