#include "module_analyzer.h"

#include "utils/vector_utils.h"

namespace pickc::pcir
{
  Result<Register*, std::vector<std::string>> ModuleAnalyzer::fnDefAnalyze(const parser::FunctionDefineNode* fnDef, FlowNode** flow)
  {
    using namespace parser;
    std::vector<std::string> errors;
    auto fn = new Function();
    fn->belong = *flow;
    TypeFunction type;
    type.retType = new Type(fnDef->retType);

    for(uint32_t index = 0, l = fnDef->args.size(); index < l; ++index) {
      auto argDef = fnDef->args[index];
      type.args.push_back(new Type(argDef->type));
      if(std::find(fn->args.begin(), fn->args.end(), argDef->name->name) != fn->args.end()) {
        errors.push_back(createSemanticError(argDef, "引数名 " + argDef->name->name + " が重複しています。"));
      }
      auto arg = new Register();
      arg->mut = argDef->isMut ? Mutability::Mutable : Mutability::Immutable;
      arg->status = RegisterStatus::InUse;
      arg->vType = ValueType::LeftValue;
      arg->type = Type(argDef->type);
      arg->scope = RegisterScope::Argument;
      fn->args.push_back(argDef->name->name);
      fn->regs.push_back(arg);
      fn->entryFlow->vars[argDef->name->name] = arg;
      if(argDef->init) {
        fn->defaultArgs[argDef->name->name].flow = new FlowNode();
        fn->defaultArgs[argDef->name->name].flow->belong = fn;
        if(auto defaultArg = exprAnalyze(argDef->init, &fn->defaultArgs[argDef->name->name].flow)) {
          fn->defaultArgs[argDef->name->name].reg = defaultArg.get();
        }
        else errors += defaultArg.err();
      }

      auto loadArg = new LoadArgInstruction();
      loadArg->reg = arg;
      loadArg->indexOfArg = index;
      fn->entryFlow->insts.push_back(loadArg);
    }
    if(!errors.empty()) {
      delete fn;
      return error(errors);
    }

    fn->type = type;
    if(auto body = exprAnalyze(fnDef->body, &fn->entryFlow)) {
      if(body.get()) {
        if(!Type::castable(*fn->type.fn.retType, body.get()->type)) {
          errors.push_back(createSemanticError(fnDef, "関数は " + fn->type.fn.retType->toString() + " を返しますが、" + body.get()->type.toString() + " が返されました。"));
        }
        *fn->type.fn.retType = Type::merge(Mov, *fn->type.fn.retType, body.get()->type);
        auto ret = new ReturnInstruction();
        ret->reg = body.get();
        fn->flows.back()->insts.push_back(ret);
      }
    }
    else errors += body.err();
    
    if(errors.empty()) {
      auto reg = new Register();
      reg->status = RegisterStatus::InUse;
      reg->type = fn->type;
      reg->scope = RegisterScope::LocalVariable;
      if(fnDef->name) {
        if((*flow)->findVar(fnDef->name->name)) errors.push_back(createSemanticError(fnDef->name, "既に変数 " + fnDef->name->name + " は定義されています。"));
        reg->mut = Mutability::Immutable;
        reg->vType = ValueType::LeftValue;
        (*flow)->vars[fnDef->name->name] = reg;
      }
      else {
        reg->mut = Mutability::Mutable;
        reg->vType = ValueType::RightValue;
      }
      (*flow)->addReg(reg);
      auto inst = new LoadFnInstruction();
      inst->reg = reg;
      inst->fn = fn;
      (*flow)->insts.push_back(inst);
      tree->module.functions.push_back(fn);
      return ok(reg);
    }
    delete fn;
    return error(errors);
  }
}