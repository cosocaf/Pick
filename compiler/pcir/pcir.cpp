#include "pcir.h"

#include "pickc/config.h"
#include "utils/instanceof.h"
#include "pcir_format.h"

namespace pickc::pcir
{
  Register* FlowNode::findVar(const std::string& name) const
  {
    if(vars.find(name) != vars.end()) return vars.at(name);
    if(parentFlow) return parentFlow->findVar(name);
    if(belong->belong) return belong->belong->findVar(name);
    // TODO: find global vars
    return nullptr;
  }
  Option<std::string> FlowNode::retNotice(const Type& t) const
  {
    return belong->retNotice(t);
  }
  void FlowNode::addReg(Register* reg)
  {
    belong->addReg(reg);
  }
  Function::Function() : entryFlow(new FlowNode()), result(nullptr), flows({ entryFlow }), belong(nullptr)
  {
    entryFlow->belong = this;
  }
  Option<std::string> Function::retNotice(const Type& t) const
  {
    if(!Type::castable(*type.fn.retType, t)) {
      return some("関数は " + type.fn.retType->toString() + " を返しますが、" + t.toString() + " が返されました。");
    }
    *type.fn.retType = Type::merge(Ret, *type.fn.retType, t);
    return none;
  }
  void Function::addReg(Register* reg)
  {
    assert(reg != nullptr);
    regs.push_back(reg);
  }
  Type::Type() : type(Types::_UNDEFINED) {}
  Type::Type(Types type) : type(type)
  {
    assert(
      type != Types::Array &&
      type != Types::Ptr &&
      type != Types::Function &&
      type != Types::UserDefine &&
      type != Types::Generics
    );
  }
  Type::Type(const TypeArray& array) : type(Types::Array), array(array) {}
  Type::Type(const TypePtr& ptr) : type(Types::Ptr), ptr(ptr) {}
  Type::Type(const TypeFunction& fn) : type(Types::Function), fn(fn) {}
  Type::Type(const TypeUserDefine& user) : type(Types::UserDefine), user(user) {}
  Type::Type(const TypeGenerics& gen) : type(Types::Generics), gen(gen) {}
  Type::Type(parser::TypeNode* typeNode)
  {
    using namespace parser;
    if(typeNode == nullptr) type = Types::Any;
    else if (instanceof<I8TypeNode>(typeNode)) type = Types::I8;
    else if (instanceof<I16TypeNode>(typeNode)) type = Types::I16;
    else if (instanceof<I32TypeNode>(typeNode)) type = Types::I32;
    else if (instanceof<I64TypeNode>(typeNode)) type = Types::I64;
    else if (instanceof<U8TypeNode>(typeNode)) type = Types::U8;
    else if (instanceof<U16TypeNode>(typeNode)) type = Types::U16;
    else if (instanceof<U32TypeNode>(typeNode)) type = Types::U32;
    else if (instanceof<U64TypeNode>(typeNode)) type = Types::U64;
    else if (instanceof<F32TypeNode>(typeNode)) type = Types::F32;
    else if (instanceof<F64TypeNode>(typeNode)) type = Types::F64;
    else if (instanceof<VoidTypeNode>(typeNode)) type = Types::Void;
    else if (instanceof<BoolTypeNode>(typeNode)) type = Types::Bool;
    else if (instanceof<CharTypeNode>(typeNode)) type = Types::Char;
    else if (instanceof<ArrayTypeNode>(typeNode)) {
      type = Types::Array;
      auto arrayNode = dynamic_cast<ArrayTypeNode*>(typeNode);
      new (&array) TypeArray();
      array.elem = new Type(arrayNode->elemType);
      array.length = static_cast<uint32_t>(arrayNode->length);
    }
    else if (instanceof<PtrTypeNode>(typeNode)) {
      type = Types::Ptr;
      auto ptrNode = dynamic_cast<PtrTypeNode*>(typeNode);
      new (&ptr) TypePtr();
      ptr.elem = new Type(ptrNode->base);
    }
    else if (instanceof<FnTypeNode>(typeNode)) {
      type = Types::Function;
      auto fnNode = dynamic_cast<FnTypeNode*>(typeNode);
      new (&fn) TypeFunction();
      fn.retType = new Type(fnNode->ret);
      fn.args.reserve(fnNode->args.size());
      for(auto arg : fnNode->args) {
        fn.args.push_back(new Type(arg));
      }
    }
    else if (instanceof<UserDefineTypeNode>(typeNode)) {
      type = Types::UserDefine;
      // TODO
      assert(false);
    }
    else if (instanceof<GenericsTypeNode>(typeNode)) {
      type = Types::Generics;
      // TODO
      assert(false);
    }
    else {
      assert(false);
    }
  }
  Type::Type(const Type& type)
  {
    this->type = type.type;
    switch (this->type) {
      case Types::Array:
        new (&array) TypeArray(type.array);
        break;
      case Types::Ptr:
        new (&ptr) TypePtr(type.ptr);
        break;
      case Types::Function:
        new (&fn) TypeFunction(type.fn);
        break;
      case Types::UserDefine:
        new (&user) TypeUserDefine(type.user);
        break;
      case Types::Generics:
        new (&gen) TypeGenerics(type.gen);
        break;
    }
  }
  Type& Type::operator=(const Type& t)
  {
    this->type = t.type;
    switch (this->type) {
      case Types::Array:
        new (&array) TypeArray(t.array);
        break;
      case Types::Ptr:
        new (&ptr) TypePtr(t.ptr);
        break;
      case Types::Function:
        new (&fn) TypeFunction(t.fn);
        break;
      case Types::UserDefine:
        new (&user) TypeUserDefine(t.user);
        break;
      case Types::Generics:
        new (&gen) TypeGenerics(t.gen);
        break;
    }
    return *this;
  }
  Type::~Type() {
    switch (this->type) {
      case Types::Array:
        array.~TypeArray();
        break;
      case Types::Ptr:
        ptr.~TypePtr();
        break;
      case Types::Function:
        fn.~TypeFunction();
        break;
      case Types::UserDefine:
        user.~TypeUserDefine();
        break;
      case Types::Generics:
        gen.~TypeGenerics();
        break;
    }
    type = Types::_UNDEFINED;
  }
  bool Type::operator<(const Type& t) const
  {
    return toString() < t.toString();
  }
  bool Type::castable(const Type& to, const Type& from)
  {
    // TODO
    assert(to.type != Types::_UNDEFINED && from.type != Types::_UNDEFINED);
    return to.type == from.type || to.type == Types::Any;
  }
  bool Type::computable(uint8_t inst, const Type* left, const Type* right)
  {
    assert(left != nullptr);
    // TODO ユーザー定義型
    switch(inst) {
      case Add:
      case Sub:
      case Mul:
      case Div:
      case Mod:
        assert(right != nullptr);
        return left->type == right->type && (left->isInt() || left->isFloat())
        && (right->isInt() || right->isFloat());
      case Inc:
      case Dec:
      case Pos:
      case Neg:
        assert(right == nullptr);
        return left->isInt() || left->isFloat();
      default:
        assert(false);
        return false;
    }
  }
  Type Type::merge(uint8_t inst, const Type& t1, const Type& t2)
  {
    if(t1.type == Types::Any) return t2;
    switch(inst) {
      case Add:
      case Sub:
      case Mul:
      case Div:
      case Mod:
        if((t1.isFloat() || t2.isFloat()) && (t1.isFloat() || t1.isInt() || t2.isFloat() || t2.isInt())) {
          if(t1.type == Types::F64 || t2.type == Types::F64) {
            return Type(Types::F64);
          }
          else if(t1.type == Types::F32 || t2.type == Types::F32) {
            return Type(Types::F32);
          }
          return Type(Types::Float);
        }
        if(t1.type == Types::Integer && t2.type == Types::Integer) return Type(Types::Integer);
        if(t1.isInt() && t2.isInt()) {
          if(t1.size() == t2.size()) {
            if(t1.isUnsignedInt()) return t1;
            if(t2.isUnsignedInt()) return t2;
            return t1;
          }
          return t1.size() > t2.size() ? t1 : t2;
        }
        assert(false);
        break;
      case Ret:
      case Mov:
        assert(castable(t1, t2));
        return t1;
      default:
        assert(false);
    }
    return Type();
  }
  bool Type::isAny() const
  {
    return type == Types::Any;
  }
  bool Type::isInt() const
  {
    return type == Types::Integer || isSignedInt() || isUnsignedInt();
  }
  bool Type::isSignedInt() const
  {
    return type == Types::I8 || type == Types::I16 || type == Types::I32 || type == Types::I64;
  }
  bool Type::isUnsignedInt() const
  {
    return type == Types::U8 || type == Types::U16 || type == Types::U32 || type == Types::U64;
  }
  bool Type::isFloat() const
  {
    return type == Types::Float || type == Types::F32 || type == Types::F64;
  }
  bool Type::isArray() const
  {
    return type == Types::Array;
  }
  bool Type::isVoid() const
  {
    return type == Types::Void;
  }
  bool Type::isBool() const
  {
    return type == Types::Bool;
  }
  bool Type::isChar() const
  {
    return type == Types::Char;
  }
  bool Type::isPtr() const
  {
    return type == Types::Ptr;
  }
  bool Type::isFn() const
  {
    return type == Types::Function;
  }
  bool Type::isUserDef() const
  {
    return type == Types::UserDefine;
  }
  bool Type::isGenerics() const
  {
    return type == Types::Generics;
  }
  uint32_t Type::size() const
  {
    switch(type) {
      case Types::Integer: return 32;
      case Types::Float: return 64;
      case Types::I8: return 8;
      case Types::I16: return 16;
      case Types::I32: return 32;
      case Types::I64: return 64;
      case Types::U8: return 8;
      case Types::U16: return 16;
      case Types::U32: return 32;
      case Types::U64: return 64;
      case Types::F32: return 32;
      case Types::F64: return 64;
      case Types::Void: return 0;
      case Types::Bool: return 1;
      case Types::Char: return 8;
      case Types::Array: return array.elem->size() * array.length;
      case Types::Ptr: return 0;
      case Types::Function: return 0;
      case Types::UserDefine: assert(false); return 0; // TODO
      case Types::Generics: assert(false); return 0; // TODO
      default: assert(false); return static_cast<uint32_t>(-1);
    }
  }
  std::string Type::toString() const
  {
    switch(type) {
      case Types::Any: return "any";
      case Types::Integer: return "i32";
      case Types::Float: return "f64";
      case Types::I8: return "i8";
      case Types::I16: return "i16";
      case Types::I32: return "i32";
      case Types::I64: return "i64";
      case Types::U8: return "u8";
      case Types::U16: return "u16";
      case Types::U32: return "u32";
      case Types::U64: return "u64";
      case Types::F32: return "f32";
      case Types::F64: return "f64";
      case Types::Void: return "void";
      case Types::Bool: return "bool";
      case Types::Char: return "char";
      case Types::Array: return "[" + array.elem->toString() + " x " + std::to_string(array.length) + "]";
      case Types::Ptr: return "ptr<" + ptr.elem->toString() + ">";
      case Types::Function: {
        std::string result = "fn(";
        if(!fn.args.empty()) {
          result += fn.args[0]->toString();
          for(size_t i = 1, l = fn.args.size(); i < l; ++i) {
            result += ',';
            result += fn.args[i]->toString();
          }
        }
        result += "):";
        result += fn.retType->toString();
        return result;
      }
      case Types::UserDefine: assert(false); return ""; // TODO
      case Types::Generics: assert(false); return ""; // TODO
      default: assert(false); return "";
    }
  }
  Instruction::~Instruction() {}
}