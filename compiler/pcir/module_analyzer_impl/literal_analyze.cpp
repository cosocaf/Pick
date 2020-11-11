#include "module_analyzer.h"

#include "utils/instanceof.h"
#include "utils/vector_utils.h"
#include "utils/dyn_cast.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::literalAnalyze(const parser::LiteralNode* literal, FlowNode** flow)
  {
    using namespace parser;
    auto reg = new Register();
    (*flow)->addReg(reg);
    if(instanceof<IntegerLiteral>(literal)) {
      reg->type = Type(Types::Integer);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i32 = dynCast<IntegerLiteral>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I8Literal>(literal)) {
      reg->type = Type(Types::I8);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i8 = dynCast<I8Literal>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I16Literal>(literal)) {
      reg->type = Type(Types::I16);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i16 = dynCast<I16Literal>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I32Literal>(literal)) {
      reg->type = Type(Types::I32);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i32 = dynCast<I32Literal>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<I64Literal>(literal)) {
      reg->type = Type(Types::I64);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.i64 = dynCast<I64Literal>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<NullLiteral>(literal)) {
      reg->type = Type(Types::Null);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.null = nullptr;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<CharLiteral>(literal)) {
      reg->type = Type(Types::Char);
      auto inst = new ImmMove();
      inst->dist = reg;
      inst->type = reg->type;
      inst->imm.c = dynCast<CharLiteral>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else if(instanceof<StringLiteral>(literal)) {
      reg->type = Type(TypePtr{ new Type(Types::Char) });
      auto inst = new LoadStringInstruction();
      inst->reg = reg;
      inst->value = dynCast<StringLiteral>(literal)->value;
      (*flow)->insts.push_back(inst);
    }
    else {
      assert(false);
    }
    return ok(reg);
  }
}