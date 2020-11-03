#ifndef PICKC_PARSER_AST_NODE_H_
#define PICKC_PARSER_AST_NODE_H_

#include <vector>
#include <string>

#include "token.h"

namespace pickc::parser
{
  class Node
  {
  public:
    std::vector<Token> tokens;
  public:
    virtual ~Node();
    virtual void dump(const std::string& indent, const std::string& indent2) const = 0;
  };
  class TypeNode : public Node {};
  class I8TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I16TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I32TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I64TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U8TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U16TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U32TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U64TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class F32TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class F64TypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class CharTypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class BoolTypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class VoidTypeNode : public TypeNode
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class PtrTypeNode : public TypeNode
  {
  public:
    TypeNode* base;
  public:
    PtrTypeNode(TypeNode* base);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ArrayTypeNode : public TypeNode
  {
  public:
    TypeNode* elemType;
    size_t length;
  public:
    ArrayTypeNode(TypeNode* elemType, size_t length);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FnTypeNode : public TypeNode
  {
  public:
    TypeNode* ret;
    std::vector<TypeNode*> args;
  public:
    FnTypeNode(TypeNode* ret, const std::vector<TypeNode*>& args);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class GenericsTypeNode : public TypeNode
  {
  public:
    std::vector<std::string> name;
    std::vector<TypeNode*> generics;
  public:
    GenericsTypeNode(const std::vector<std::string>& name, const std::vector<TypeNode*>& generics);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class UserDefineTypeNode : public TypeNode
  {
  public:
    std::vector<std::string> name;
  public:
    UserDefineTypeNode(const std::vector<std::string>& name);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ExpressionNode : public Node {};
  class PrimaryNode : public ExpressionNode {};
  class BlockNode : public PrimaryNode
  {
  public:
    std::vector<Node*> nodes;
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class VariableNode : public PrimaryNode
  {
  public:
    std::string name;
    std::vector<TypeNode*> generics;
  public:
    VariableNode(const std::string& name, const std::vector<TypeNode*>& generics);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ScopedVariableNode : public VariableNode
  {
  public:
    VariableNode* child;
  public:
    ScopedVariableNode(const std::string& name, const std::vector<TypeNode*>& generics, VariableNode* child);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class LiteralNode : public PrimaryNode {};
  class IntegerLiteral : public LiteralNode
  {
  public:
    int value;
  public:
    IntegerLiteral(int value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I8Literal : public LiteralNode
  {
  public:
    int8_t value;
  public:
    I8Literal(int8_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I16Literal : public LiteralNode
  {
  public:
    int16_t value;
  public:
    I16Literal(int16_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I32Literal : public LiteralNode
  {
  public:
    int32_t value;
  public:
    I32Literal(int32_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class I64Literal : public LiteralNode
  {
  public:
    int64_t value;
  public:
    I64Literal(int64_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U8Literal : public LiteralNode
  {
  public:
    uint8_t value;
  public:
    U8Literal(uint8_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U16Literal : public LiteralNode
  {
  public:
    uint16_t value;
  public:
    U16Literal(uint16_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U32Literal : public LiteralNode
  {
  public:
    uint32_t value;
  public:
    U32Literal(uint32_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class U64Literal : public LiteralNode
  {
  public:
    uint64_t value;
  public:
    U64Literal(uint64_t value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FloatLiteral : public LiteralNode
  {
  public:
    double value;
  public:
    FloatLiteral(double value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class F32Literal : public LiteralNode
  {
  public:
    float value;
  public:
    F32Literal(float value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class F64Literal : public LiteralNode
  {
  public:
    double value;
  public:
    F64Literal(double value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class BoolLiteral : public LiteralNode
  {
  public:
    bool value;
  public:
    BoolLiteral(bool value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class CharLiteral : public LiteralNode
  {
  public:
    char value;
  public:
    CharLiteral(char value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class StringLiteral : public LiteralNode
  {
  public:
    std::string value;
  public:
    StringLiteral(const std::string& value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ArrayLiteral : public LiteralNode
  {
  public:
    std::vector<ExpressionNode*> value;
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class UnaryNode : public ExpressionNode
  {
  public:
    ExpressionNode* base;
  public:
    UnaryNode(ExpressionNode* base);
  };
  class BinaryNode : public ExpressionNode
  {
  public:
    ExpressionNode* left;
    ExpressionNode* right;
  public:
    BinaryNode(ExpressionNode* left, ExpressionNode* right);
  };
  class BackUnaryNode : public UnaryNode {
    using UnaryNode::UnaryNode;
  };
  class BackIncrementNode : public BackUnaryNode
  {
  public:
    using BackUnaryNode::BackUnaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class BackDecrementNode : public BackUnaryNode
  {
  public:
    using BackUnaryNode::BackUnaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class MemberAccessNode : public BackUnaryNode
  {
  public:
    VariableNode* member;
  public:
    MemberAccessNode(ExpressionNode* base, VariableNode* member);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ArrayAccessNode : public BackUnaryNode
  {
  public:
    ExpressionNode* suffix;
  public:
    ArrayAccessNode(ExpressionNode* base, ExpressionNode* suffix);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class CallNode : public BackUnaryNode
  {
  public:
    std::vector<ExpressionNode*> args;
  public:
    CallNode(ExpressionNode* base, std::vector<ExpressionNode*> args);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FrontUnaryNode : public UnaryNode {
  public:
    using UnaryNode::UnaryNode;
  };
  class PlusNode : public FrontUnaryNode
  {
  public:
    using FrontUnaryNode::FrontUnaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class MinusNode : public FrontUnaryNode
  {
  public:
    using FrontUnaryNode::FrontUnaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FrontIncrementNode : public FrontUnaryNode
  {
  public:
    using FrontUnaryNode::FrontUnaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FrontDecrementNode : public FrontUnaryNode
  {
  public:
    using FrontUnaryNode::FrontUnaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FactorNode : public BinaryNode {
  public:
    using BinaryNode::BinaryNode;
  };
  class MulNode : public FactorNode
  {
  public:
    using FactorNode::FactorNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class DivNode : public FactorNode
  {
  public:
    using FactorNode::FactorNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ModNode : public FactorNode
  {
  public:
    using FactorNode::FactorNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class TermNode : public BinaryNode {
  public:
    using BinaryNode::BinaryNode;
  };
  class AddNode : public TermNode
  {
  public:
    using TermNode::TermNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class SubNode : public TermNode
  {
  public:
    using TermNode::TermNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ComparisonNode : public BinaryNode {
  public:
    using BinaryNode::BinaryNode;
  };
  class EqualNode : public ComparisonNode
  {
  public:
    using ComparisonNode::ComparisonNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class NotEqualNode : public ComparisonNode
  {
  public:
    using ComparisonNode::ComparisonNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class GreaterEqualNode : public ComparisonNode
  {
  public:
    using ComparisonNode::ComparisonNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class GreaterThanNode : public ComparisonNode
  {
  public:
    using ComparisonNode::ComparisonNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class LessEqualNode : public ComparisonNode
  {
  public:
    using ComparisonNode::ComparisonNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class LessThanNode : public ComparisonNode
  {
  public:
    using ComparisonNode::ComparisonNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class AsignNode : public BinaryNode
  {
  public:
    using BinaryNode::BinaryNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class AddAsignNode : public AsignNode
  {
  public:
    using AsignNode::AsignNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class SubAsignNode : public AsignNode
  {
  public:
    using AsignNode::AsignNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class MulAsignNode : public AsignNode
  {
  public:
    using AsignNode::AsignNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class DivAsignNode : public AsignNode
  {
  public:
    using AsignNode::AsignNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ModAsignNode : public AsignNode
  {
  public:
    using AsignNode::AsignNode;
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ReturnNode : public Node
  {
  public:
    ExpressionNode* value;
  public:
    ReturnNode(ExpressionNode* value);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class VariableDefineNode : public ExpressionNode
  {
  public:
    bool isPub;
    bool isMut;
    VariableNode* name;
    TypeNode* type;
    ExpressionNode* init;
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ArgumentDefineNode : public Node
  {
  public:
    bool isMut;
    VariableNode* name;
    TypeNode* type;
    ExpressionNode* init;
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class FunctionDefineNode : public ExpressionNode
  {
  public:
    bool isPub;
    VariableNode* name;
    std::vector<ArgumentDefineNode*> args;
    TypeNode* retType;
    ExpressionNode* body;
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ClassDefineNode : public Node
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class AliasDefineNode : public Node
  {
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ImportNode : public Node
  {
  public:
    VariableNode* name;
  public:
    ImportNode(VariableNode* name);
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class ExternNode : public Node
  {
  public:
    VariableNode* name;
    std::vector<ArgumentDefineNode*> args;
    TypeNode* retType;
  public:
    virtual void dump(const std::string& indent, const std::string& indent2) const override;
  };
  class RootNode : public Node
  {
  public:
    std::vector<Node*> nodes;
  public:
    virtual void dump(const std::string& indent = "/", const std::string& indent2 = "   ") const override;
  };
}

#endif // PICKC_PARSER_AST_NODE_H_