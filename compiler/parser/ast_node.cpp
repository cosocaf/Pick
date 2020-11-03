#include "ast_node.h"

#include <iostream>

namespace pickc::parser
{
  Node::~Node() {}
  void BlockNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Block" << std::endl;
    for(size_t i = 0, l = nodes.size(); i < l; ++i) {
      nodes[i]->dump(indent2 + "+--", indent2 + (i + 1 < l ? "|  " : "   "));
    }
  }
  IntegerLiteral::IntegerLiteral(int value) : value(value) {}
  void IntegerLiteral::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Integer Literal (" << value << ")" << std::endl;
  }
  I8Literal::I8Literal(int8_t value) : value(value) {}
  void I8Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "I8 Literal (" << static_cast<int>(value) << ")" << std::endl;
  }
  I16Literal::I16Literal(int16_t value) : value(value) {}
  void I16Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "I16 Literal (" << value << ")" << std::endl;
  }
  I32Literal::I32Literal(int32_t value) : value(value) {}
  void I32Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "I32 Literal (" << value << ")" << std::endl;
  }
  I64Literal::I64Literal(int64_t value) : value(value) {}
  void I64Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "I64 Literal (" << value << ")" << std::endl;
  }
  U8Literal::U8Literal(uint8_t value) : value(value) {}
  void U8Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "U8 Literal (" << static_cast<unsigned>(value) << ")" << std::endl;
  }
  U16Literal::U16Literal(uint16_t value) : value(value) {}
  void U16Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "U16 Literal (" << value << ")" << std::endl;
  }
  U32Literal::U32Literal(uint32_t value) : value(value) {}
  void U32Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "U32 Literal (" << value << ")" << std::endl;
  }
  U64Literal::U64Literal(uint64_t value) : value(value) {}
  void U64Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "U64 Literal (" << value << ")" << std::endl;
  }
  FloatLiteral::FloatLiteral(double value) : value(value) {}
  void FloatLiteral::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Float Literal (" << value << ")" << std::endl;
  }
  F32Literal::F32Literal(float value) : value(value) {}
  void F32Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "F32 Literal (" << value << ")" << std::endl;
  }
  F64Literal::F64Literal(double value) : value(value) {}
  void F64Literal::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "F64 Literal (" << value << ")" << std::endl;
  }
  BoolLiteral::BoolLiteral(bool value) : value(value) {}
  void BoolLiteral::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Bool Literal (" << (value ? "true" : "false") << ")" << std::endl;
  }
  CharLiteral::CharLiteral(char value) : value(value) {}
  void CharLiteral::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Char Literal (" << value << ")" << std::endl;
  }
  StringLiteral::StringLiteral(const std::string& value) : value(value) {}
  void StringLiteral::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "String Literal (" << value << ")" << std::endl;
  }
  void ArrayLiteral::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Array Literal" << std::endl;
    for(size_t i = 0, l = value.size(); i < l; ++i) {
      value[i]->dump(indent2 + "+--", indent2 + (i + 1 < l ? "|  " : "   "));
    }
  }
  VariableNode::VariableNode(const std::string& name, const std::vector<TypeNode*>& generics) : name(name), generics(generics) {}
  void VariableNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Variable" << std::endl;
    std::cout << indent2 << "+--Name" << std::endl;
    std::cout << indent2 << (generics.empty() ? "   " : "|  ") << "+--" << name << std::endl;
    if(!generics.empty()) {
      std::cout << indent2 << "+--Generics" << std::endl;
      for(size_t i = 0, l = generics.size(); i < l; ++i) {
        generics[i]->dump(indent2 + "   +--", indent2 + "   " + (i + 1 < l ? "|  " : "   "));
      }
    }
  }
  ScopedVariableNode::ScopedVariableNode(const std::string& name, const std::vector<TypeNode*>& generics, VariableNode* child) : VariableNode(name, generics), child(child) {}
  void ScopedVariableNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Scoped Variable (" << name[0];
    for(size_t i = 1, l = name.size(); i < l; ++i) {
      std::cout << '::' << name[i];
    }
    if(child) child->dump(indent2 + "+--", indent2 + "   ");
    std::cout << ')' << std::endl;
  }
  void I8TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type I8" << std::endl;
  }
  void I16TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type I16" << std::endl;
  }
  void I32TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type I32" << std::endl;
  }
  void I64TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type I64" << std::endl;
  }
  void U8TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type U8" << std::endl;
  }
  void U16TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type U16" << std::endl;
  }
  void U32TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type U32" << std::endl;
  }
  void U64TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type U64" << std::endl;
  }
  void F32TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type F32" << std::endl;
  }
  void F64TypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type F64" << std::endl;
  }
  void CharTypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type Char" << std::endl;
  }
  void BoolTypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type Bool" << std::endl;
  }
  void VoidTypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type Void" << std::endl;
  }
  PtrTypeNode::PtrTypeNode(TypeNode* base) : base(base) {}
  void PtrTypeNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Type Ptr" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  ArrayTypeNode::ArrayTypeNode(TypeNode* elemType, size_t length) : elemType(elemType), length(length) {}
  void ArrayTypeNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Type Array" << std::endl;
    std::cout << indent2 << "+--Elem Type" << std::endl;
    elemType->dump(indent2 + "|  +--", indent2 + "|     ");
    std::cout << indent2 << "+--Size " << length << std::endl;
  }
  FnTypeNode::FnTypeNode(TypeNode* ret, const std::vector<TypeNode*>& args) : ret(ret), args(args) {}
  void FnTypeNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Type Function" << std::endl;
    std::cout << indent2 << "+--Return Type" << std::endl;
    ret->dump(indent2 + "|  +--", indent2 + "|     ");
    std::cout << indent2 << "+--Args" << std::endl;
    for(size_t i = 0, l = args.size(); i < l; ++i) {
      args[i]->dump(indent2 + "   +--", indent2 + "   " + (i + 1 < l ? "|  " : "   "));
    }
  }
  GenericsTypeNode::GenericsTypeNode(const std::vector<std::string>& name, const std::vector<TypeNode*>& generics) : name(name), generics(generics) {}
  void GenericsTypeNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Type Generics (" << name[0];
    for(size_t i = 1, l = name.size(); i < l; ++i) std::cout << "::" << name[i];
    std::cout << ")" << std::endl;
    for(size_t i = 0, l = generics.size(); i < l; ++i) {
      generics[i]->dump(indent2 + "+--", indent2 + (i + 1 < l ? "|  " : "   "));
    }
  }
  UserDefineTypeNode::UserDefineTypeNode(const std::vector<std::string>& name) : name(name) {}
  void UserDefineTypeNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Type User Define (" << name[0];
    for(size_t i = 1, l = name.size(); i < l; ++i) std::cout << "::" << name[i];
    std::cout << ")" << std::endl;
  }
  UnaryNode::UnaryNode(ExpressionNode* base) : base(base) {}
  BinaryNode::BinaryNode(ExpressionNode* left, ExpressionNode* right) : left(left), right(right) {}
  void BackIncrementNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Back Increment" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  void BackDecrementNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Back Decrement" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  MemberAccessNode::MemberAccessNode(ExpressionNode* base, VariableNode* member) : BackUnaryNode(base), member(member) {}
  void MemberAccessNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Member Access" << std::endl;
    std::cout << indent2 << "+--Base" << std::endl;
    base->dump(indent2 + "|  +--", indent2 + "|     ");
    std::cout << indent2 << "+--Member" << std::endl;
    member->dump(indent2 + "   +--", indent2 + "      ");
  }
  ArrayAccessNode::ArrayAccessNode(ExpressionNode* base, ExpressionNode* suffix) : BackUnaryNode(base), suffix(suffix) {}
  void ArrayAccessNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Array Access" << std::endl;
    std::cout << indent2 << "+--Base" << std::endl;
    base->dump(indent2 + "|  +--", indent2 + "|     ");
    std::cout << indent2 << "+--Suffix" << std::endl;
    suffix->dump(indent2 + "   +--", indent2 + "      ");
  }
  CallNode::CallNode(ExpressionNode* base, std::vector<ExpressionNode*> args) : BackUnaryNode(base), args(args) {}
  void CallNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Call" << std::endl;
    std::cout << indent2 << "+--Function" << std::endl;
    base->dump(indent2 + "|  +--", indent2 + "|     ");
    std::cout << indent2 << "+--Args" << std::endl;
    for(size_t i = 0, l = args.size(); i < l; ++i) {
      args[i]->dump(indent2 + "   +--", indent2 + "   " + (i + 1 < l ? "|  " : "   "));
    }
  }
  void PlusNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Plus" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  void MinusNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Minus" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  void FrontIncrementNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Front Increment" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  void FrontDecrementNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Front Decrement" << std::endl;
    base->dump(indent2 + "+--", indent2 + "   ");
  }
  namespace
  {
    inline void binaryDump(const std::string& indent, const std::string& indent2, const std::string& op, const ExpressionNode* left, const ExpressionNode* right)
    {
      std::cout << indent << op << std::endl;
      std::cout << indent2 << "+--Left" << std::endl;
      left->dump(indent2 + "|  +--", indent2 + "|     ");
      std::cout << indent2 << "+--Right" << std::endl;
      right->dump(indent2 + "   +--", indent2 + "      ");
    }
  }
  void MulNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Mul", left, right);
  }
  void DivNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Div", left, right);
  }
  void ModNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent , indent2, "Mod", left, right);
  }
  void AddNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Add", left, right);
  }
  void SubNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Sub", left, right);
  }
  void EqualNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Equal", left, right);
  }
  void NotEqualNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Not Equal", left, right);
  }
  void GreaterEqualNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Greater Equal", left, right);
  }
  void GreaterThanNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Greater Than", left, right);
  }
  void LessEqualNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Less Equal", left, right);
  }
  void LessThanNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Less Than", left, right);
  }
  void AsignNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Asign", left, right);
  }
  void AddAsignNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Add Asign", left, right);
  }
  void SubAsignNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Sub Asign", left, right);
  }
  void MulAsignNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Mul Asign", left, right);
  }
  void DivAsignNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Div Asign", left, right);
  }
  void ModAsignNode::dump(const std::string& indent, const std::string& indent2) const
  {
    binaryDump(indent, indent2, "Mod Asign", left, right);
  }
  ReturnNode::ReturnNode(ExpressionNode* value) : value(value) {}
  void ReturnNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Return" << std::endl;
    if(value) {
      value->dump(indent2 + "+--", indent2 + "   ");
    }
  }
  void VariableDefineNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Variable Define" << std::endl;
    std::cout << indent2 << "+--Name" << std::endl;
    name->dump(indent2 + ((type || init) ? "|  " : "   ") + "+--", indent2 + ((type || init) ? "|  " : "   ") + "   ");
    if(type) {
      std::cout << indent2 << "+--Type" << std::endl;
      type->dump(indent2 + (init ? "|  " : "   ") + "+--", indent2 + (init ? "|  " : "   ") + "   ");
    }
    if(init) {
      std::cout << indent2 << "+--Init" << std::endl;
      init->dump(indent2 + "   +--", indent2 + "      ");
    }
  }
  void ArgumentDefineNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Argument Define" << std::endl;
    std::cout << indent2 << "+--Name" << std::endl;
    name->dump(indent2 + ((type || init) ? "|  " : "   ") + "+--", indent2 + ((type || init) ? "|  " : "   ") + "   ");
    if(type) {
      std::cout << indent2 << "+--Type" << std::endl;
      type->dump(indent2 + (init ? "|  " : "   ") + "+--", indent2 + (init ? "|  " : "   ") + "   ");
    }
    if(init) {
      std::cout << indent2 << "+--Init" << std::endl;
      init->dump(indent2 + "   +--", indent2 + "      ");
    }
  }
  void FunctionDefineNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Function Define" << std::endl;
    if(name) {
      std::cout << indent2 << "+--Name" << std::endl;
      name->dump(indent2 + "|  +--", indent2 + "|     ");
    }
    std::cout << indent2 << "+--Args" << std::endl;
    for(size_t i = 0, l = args.size(); i < l; ++i) {
      args[i]->dump(indent2 + "|  +--", indent2 + (i + 1 < l ? "|  |  " : "|     "));
    }
    if(retType) {
      std::cout << indent2 << "+--Return Type" << std::endl;
      retType->dump(indent2 + "|  +--", indent2 + "|     ");
    }
    std::cout << indent2 << "+--Body" << std::endl;
    body->dump(indent2 + "   +--", indent2 + "      ");
  }
  void ClassDefineNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Class Define" << std::endl;
  }
  void AliasDefineNode::dump(const std::string& indent, const std::string&) const
  {
    std::cout << indent << "Alias Define" << std::endl;
  }
  void ImportNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Import Module" << std::endl;
    name->dump(indent2 + "+--", indent2 + "   ");
  }
  void ExternNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Extern Declare" << std::endl;
    std::cout << indent2 << "+--Name" << std::endl;
    name->dump(indent2 + "|  +--", indent2 + "|     ");
    std::cout << indent2 << "+--Args" << std::endl;
    for(const auto& arg : args) {
      arg->dump(indent2 + "|  +--", indent2 + "|     ");
    }
    std::cout << indent2 << "+--Return Type" << std::endl;
    retType->dump(indent2 + "   +--", indent2 + "      ");
  }
  void RootNode::dump(const std::string& indent, const std::string& indent2) const
  {
    std::cout << indent << "Root Node" << std::endl;
    for(size_t i = 0, l = nodes.size(); i < l; ++i) {
      nodes[i]->dump(indent2 + "+--", indent2 + (i + 1 < l ? "|  " : "   "));
    }
  }
}