#include "routine.h"

namespace pickc::windows::x64
{
  LeaveOperation::LeaveOperation() : Operation(OperationSize::DWord) {}
  BinaryVec LeaveOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return { 0xC9 };
  }
  RetOperation::RetOperation() : Operation(OperationSize::DWord) {}
  BinaryVec RetOperation::bin(WindowsX64& x64, Routine* routine)
  {
    return { 0xC3 };
  }
}