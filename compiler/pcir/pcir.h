#ifndef PICKC_PCIR_PCIR_H_
#define PICKC_PCIR_PCIR_H_

#include <map>
#include <unordered_map>
#include <unordered_set>

#include "parser/ast_node.h"
#include "pcir_code.h"

namespace pickc
{
  struct ModuleTree;
}

namespace pickc::pcir
{
  enum struct Scope
  {
    Private,
    Public,
  };
  enum struct Mutability
  {
    Immutable,
    Mutable,
  };
  enum struct Types
  {
    _UNDEFINED,
    // 未初期化変数などの、型が定まっていない状態
    Any,
    // IntegerLiteralの型。状況によってさまざまな型に変換される。
    Integer,
    // FloatLiteralの型。状況によってF32, F64のいずれかに変換される。
    Float,
    // NullLiteralの型。状況によってptr<T>のTが異なる型に変換される。
    Null,
    I8,
    I16,
    I32,
    I64,
    U8,
    U16,
    U32,
    U64,
    F32,
    F64,
    Array,
    Void,
    Ptr,
    Bool,
    Char,
    Function,
    UserDefine,
    Generics,
  };
  struct Type;
  struct TypeArray
  {
    Type* elem;
    uint32_t length;
  };
  struct TypePtr
  {
    Type* elem;
  };
  struct TypeFunction
  {
    Type* retType;
    std::vector<Type*> args;
  };
  struct TypeUserDefine
  {

  };
  struct TypeGenerics
  {

  };
  struct Type
  {
    Types type;
    union {
      TypeArray array;
      TypePtr ptr;
      TypeFunction fn;
      TypeUserDefine user;
      TypeGenerics gen;
    };
    Type();
    Type(Types type);
    Type(const TypeArray& array);
    Type(const TypePtr& ptr);
    Type(const TypeFunction& fn);
    Type(const TypeUserDefine& user);
    Type(const TypeGenerics& gen);
    Type(parser::TypeNode* typeNode);
    Type(const Type& type);
    ~Type();
    Type& operator=(const Type& type);
    // setのinsert用
    bool operator<(const Type& type) const;
    static bool castable(const Type& to, const Type& from);
    static bool computable(uint8_t inst, const Type* left, const Type* right = nullptr);
    static Type merge(uint8_t inst, const Type& t1, const Type& t2);
    bool isAny() const;
    bool isInt() const;
    bool isSignedInt() const;
    bool isUnsignedInt() const;
    bool isFloat() const;
    bool isArray() const;
    bool isVoid() const;
    bool isBool() const;
    bool isNull() const;
    bool isChar() const;
    bool isPtr() const;
    bool isFn() const;
    bool isUserDef() const;
    bool isGenerics() const;
    uint32_t size() const;
    std::string toString() const;
  };

  enum struct RegisterStatus
  {
    Uninited,
    InUse,
    Moved,
  };
  enum struct RegisterScope
  {
    LocalVariable,
    Argument,
    GlobalVariable,
  };
  struct Variable;
  enum struct ValueType
  {
    LValue,
    RValue
  };
  struct Register
  {
    Type type;
    Variable* curVar;
    ValueType vType = ValueType::LValue;
  };
  // struct RegisterX
  // {
  //   Mutability mut;
  //   ValueType vType;
  //   Type type;
  //   RegisterStatus status;
  //   RegisterScope scope;
  // };

  struct FlowNode;
  struct Function;

  struct Instruction
  {
    virtual ~Instruction() = 0;
  };
  enum struct BinaryInstructions
  {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    EQ,
    NEQ,
    GT,
    GE,
    LT,
    LE
  };
  struct BinaryInstruction : public Instruction
  {
    BinaryInstructions inst;
    Register* dist;
    Register* left;
    Register* right;
  };
  enum struct UnaryInstructions
  {
    Inc,
    Dec,
    Pos,
    Neg,
  };
  struct UnaryInstruction : public Instruction
  {
    UnaryInstructions inst;
    Register* dist;
    Register* reg;
  };
  union Immediate {
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    float f32;
    double f64;
    bool b;
    std::nullptr_t null;
    char c;
  };
  struct ImmMove : public Instruction
  {
    Register* dist;
    Type type;
    Immediate imm;
  };
  struct CallInstruction : public Instruction
  {
    Register* dist;
    Register* fn;
    std::vector<Register*> args;
  };
  struct LoadFnInstruction : public Instruction
  {
    Register* reg;
    Function* fn;
  };
  struct LoadArgInstruction : public Instruction
  {
    Register* reg;
    uint32_t indexOfArg;
  };
  struct LoadSymbolInstruction : public Instruction
  {
    Register* reg;
    std::string name;
  };
  struct LoadStringInstruction : public Instruction
  {
    Register* reg;
    std::string value;
  };
  struct LoadElemInstruction : public Instruction
  {
    Register* dist;
    Register* array;
    Register* index;
  };
  struct AllocInstruction : public Instruction
  {
    Register* dist;
    Register* src;
  };

  struct MovInstruction : public Instruction
  {
    Register* dist;
    Register* src;
  };
  struct PhiInstruction : public Instruction
  {
    Register* dist;
    Register* r1;
    Register* r2;
  };

  enum struct FunctionType
  {
    Function,
    Extern,
  };

  struct Function;
  enum struct VariableStatus
  {
    Uninited,
    InUse,
    Moved,
  };
  struct Variable
  {
    std::string name;
    Mutability mut;
    VariableStatus status;
    Type type;
    Register* reg;
  };
  enum struct FlowType
  {
    Undecided,
    Normal,
    ConditionalBranch,
    EndPoint,
  };
  struct FlowNode
  {
    FlowType type;
    Function* belong;
    FlowNode* parentFlow;
    std::vector<Instruction*> insts;
    std::unordered_set<Variable*> vars;
    FlowNode* nextFlow;
    Register* cond;
    FlowNode* thenFlow;
    FlowNode* elseFlow;
    Register* retReg;
    Register* result;
    Variable* findVar(const std::string& name) const;
    std::unordered_map<Variable*, Register*> currentVars() const;
    Option<std::string> retNotice(const Type& t) const;
    void addReg(Register* reg);
  };
  struct Function
  {
    FunctionType fType;
    std::string externName;
    struct DefaultArgument
    {
      FlowNode* flow;
      Register* reg;
    };
    Type type;
    FlowNode* entryFlow;
    std::vector<FlowNode*> flows;
    std::vector<Register*> regs;
    Register* result;
    std::vector<std::string> args;
    std::unordered_map<std::string, DefaultArgument> defaultArgs;
    FlowNode* belong;
    Option<std::string> retNotice(const Type& t) const;
    void addReg(Register* reg);
    Function();
  };

  struct Symbol
  {
    std::string name;
    std::string fullyQualifiedName;
    Type type;
    Scope scope;
    Mutability mut;
    Function* init;
    parser::ExpressionNode* expr;
  };
  struct Module
  {
    std::map<std::string, Symbol*> symbols;
    std::vector<Function*> functions;
    std::unordered_set<ModuleTree*> importModules;
  };
}

#endif // PICKC_PCIR_PCIR_H_