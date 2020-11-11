#include "module_analyzer.h"

#include "pickc/config.h"
#include "utils/vector_utils.h"
#include "utils/map_utils.h"
#include "utils/instanceof.h"
#include "utils/dyn_cast.h"

#include "semantic_analyzer.h"

namespace pickc::pcir
{
  ModuleAnalyzer::ModuleAnalyzer(SemanticAnalyzer* sa, ModuleTree* tree, const std::unordered_set<ModuleTree*>& trees) : sa(sa), tree(tree), trees(trees) {}
  std::string ModuleAnalyzer::createSemanticError(const parser::Node* node, const std::string& message)
  {
    // assert(false);
    std::string res;
    res += "エラー: ";
    res += message;
    res += "\nat ";
    res += tree->sequence.file;
    res += ' ';
    res += std::to_string(node->tokens[0].line);
    res += "行目 ";
    res += std::to_string(node->tokens[0].letter);
    res += "文字目\n";
    if(node->tokens[0].line > 1) res += tree->sequence.toOutputString(node->tokens[0].line - 2);
    auto curLine = node->tokens[0].line;
    auto beginLetter = node->tokens[0].letter - 1;
    auto endLetter = beginLetter + node->tokens[0].value.size();
    for(auto& token : node->tokens) {
      if(token.line == curLine) {
        endLetter = token.letter - 1 + token.value.size();
      }
      else {
        res += tree->sequence.toOutputString(curLine - 1);
        auto digit = 3 + beginLetter;
        {
          auto n = curLine - 1;
          while(n != 0) {
            n /= 10;
            ++digit;
          }
        }
        res += CONSOLE_FG_RED;
        res.append(digit, ' ');
        res.append(endLetter - beginLetter, '^');
        res += '\n';
        curLine = token.line;
        beginLetter = token.letter - 1;
        endLetter = beginLetter + token.value.size();
      }
    }
    res += tree->sequence.toOutputString(curLine - 1);
    auto digit = 3 + beginLetter;
    {
      auto n = curLine - 1;
      while(n != 0) {
        n /= 10;
        ++digit;
      }
    }
    res += CONSOLE_FG_RED;
    res.append(digit, ' ');
    res.append(endLetter - beginLetter, '^');
    res += '\n';
    if(node->tokens.back().line < tree->sequence.rawFileData.size()) res += tree->sequence.toOutputString(node->tokens.back().line);
    return res;
  }
  Option<std::vector<std::string>> ModuleAnalyzer::declare()
  {
    using namespace parser;
    std::vector<std::string> errors;
    for(const auto& node : tree->ast.nodes) {
      if(instanceof<FunctionDefineNode>(node)) {
        auto fn = dynCast<FunctionDefineNode>(node);
        if(fn->name == nullptr) continue;
        if(tree->module.symbols.find(fn->name->name) == tree->module.symbols.end()) {
          auto symbol = new Symbol();
          symbol->name = fn->name->name;
          symbol->fullyQualifiedName = tree->name + "::" + symbol->name;
          TypeFunction tf{};
          tf.retType = new Type(fn->retType);
          for(const auto& arg : fn->args) tf.args.push_back(new Type(arg->type));
          symbol->type = tf;
          symbol->scope = fn->isPub ? Scope::Public : Scope::Private;
          symbol->mut = Mutability::Immutable;
          symbol->expr = fn;
          tree->module.symbols[fn->name->name] = symbol;
          sa->texts.insert(symbol->fullyQualifiedName);
        }
        else errors.push_back(createSemanticError(fn->name, "既にシンボル " + fn->name->name + " は存在します。"));
      }
      else if(instanceof<VariableDefineNode>(node)) {
        auto var = dynCast<VariableDefineNode>(node);
        if(tree->module.symbols.find(var->name->name) == tree->module.symbols.end()) {
          auto symbol = new Symbol();
          symbol->name = var->name->name;
          symbol->fullyQualifiedName = tree->name + "::" + symbol->name;
          symbol->type = var->type;
          symbol->scope = var->isPub ? Scope::Public : Scope::Private;
          symbol->mut = var->isMut ? Mutability::Mutable : Mutability::Immutable;
          symbol->expr = var->init;
          tree->module.symbols[var->name->name] = symbol;
          sa->texts.insert(symbol->fullyQualifiedName);
        }
        else errors.push_back(createSemanticError(var->name, "既にシンボル " + var->name->name + " は存在します。"));
      }
      else if(instanceof<ClassDefineNode>(node)) {
        // TODO
        assert(false);
      }
      else if(instanceof<AliasDefineNode>(node)) {
        // TODO
        assert(false);
      }
      else if(instanceof<ImportNode>(node)) {
        auto imp = dynCast<ImportNode>(node);
        std::vector<std::string> module;
        auto curMod = imp->name;
        module.insert(module.begin(), curMod->name);
        while(instanceof<ScopedVariableNode>(curMod)) {
          curMod = dynCast<ScopedVariableNode>(curMod)->child;
          module.insert(module.begin(), curMod->name);
        }
        bool found = false;
        for(const auto& t : trees) {
          if(t->name == module[0]) {
            auto curTree = t;
            for(size_t i = 1, l = module.size(); i < l; ++i) {
              if(keyExists(curTree->submodules, module[i])) {
                curTree = curTree->submodules[module[i]];
              }
              else {
                curTree = nullptr;
                break;
              }
            }
            if(curTree != nullptr) {
              tree->module.importModules.insert(curTree);
              found = true;
            }
          }
          if(found) break;
        }
        if(!found) {
          std::string moduleName = module[0];
          for(size_t i = 1, l = module.size(); i < l; ++i) moduleName += "::" + module[i];
          errors.push_back(createSemanticError(imp, "モジュール " + moduleName + " が見つかりません。"));
        }
      }
      else if(instanceof<ExternNode>(node)) {
        auto ext = dynCast<ExternNode>(node);
        if(tree->module.symbols.find(ext->name->name) == tree->module.symbols.end()) {
          auto symbol = new Symbol();
          symbol->name = ext->name->name;
          symbol->fullyQualifiedName = tree->name + "::" + symbol->name;
          TypeFunction tf{};
          tf.retType = new Type(ext->retType);
          for(const auto& arg : ext->args) tf.args.push_back(new Type(arg->type));
          symbol->type = tf;
          symbol->scope = ext->isPub ? Scope::Public : Scope::Private;
          symbol->mut = Mutability::Immutable;
          symbol->expr = ext;
          tree->module.symbols[ext->name->name] = symbol;
          sa->texts.insert(symbol->fullyQualifiedName);
        }
        else errors.push_back(createSemanticError(ext->name, "既にシンボル " + ext->name->name + " は存在します。"));
      }
      else {
        assert(false);
      }
    }
    if(errors.empty()) return none;
    return some(errors);
  }
  Option<std::vector<std::string>> ModuleAnalyzer::analyze()
  {
    std::vector<std::string> errors;
    for(auto& symbol : tree->module.symbols) {
      if(!symbol.second->init) {
        symbol.second->init = new Function();
        auto curFlow = symbol.second->init->entryFlow;
        if(auto init = exprAnalyze(symbol.second->expr, &curFlow)) {
          assert(symbol.second->init->flows.size() == 1);
          if(init.get()) {
            symbol.second->init->result = init.get();
            if(!Type::castable(symbol.second->type, init.get()->type)) {
              errors.push_back(createSemanticError(symbol.second->expr, "変数の型は " + symbol.second->type.toString() + " ですが、" + init.get()->type.toString() + "が代入されました。"));
            }
            else {
              symbol.second->type = Type::merge(Mov, symbol.second->type, init.get()->type);
              TypeFunction tf{};
              tf.retType = new Type(init.get()->type);
              symbol.second->init->type = tf;
              curFlow->type = FlowType::EndPoint;
              curFlow->retReg = init.get();
            }
            tree->module.functions.push_back(symbol.second->init);
          }
          else {
            errors.push_back(createSemanticError(symbol.second->expr, "シンボル " + symbol.second->name + " は正しく初期化されません。"));
          }
        }
        else errors += init.err();
      }
    }
    if(errors.empty()) return none;
    return some(errors);
  }
  Result<Symbol*, std::vector<std::string>> ModuleAnalyzer::findGlobalVar(const parser::VariableNode* var)
  {
    using namespace parser;
    if(instanceof<ScopedVariableNode>(var)) {
      std::vector<std::string> module;
      auto cur = var;
      while(instanceof<ScopedVariableNode>(cur)) {
        cur = dynCast<ScopedVariableNode>(cur)->child;
        module.insert(module.begin(), cur->name);
      }
      std::string name = module[0];
      for(size_t i = 1, l = module.size(); i < l; ++i) name += "::" + module[i];
      for(const auto& mod : tree->module.importModules) {
        if(mod->name == name) {
          if(keyExists(mod->module.symbols, var->name)) {
            if(mod->module.symbols[var->name]->scope == Scope::Public) {
              return ok(mod->module.symbols[var->name]);
            }
            else {
              return error(std::vector{ createSemanticError(var, "変数 " + name + "::" + var->name + " にアクセスできません。変数はプライベートです。") });
            }
          }
        }
      }
    }
    else {
      if(keyExists(tree->module.symbols, var->name)) {
        return ok(tree->module.symbols[var->name]);
      }
    }
    return error(std::vector{ createSemanticError(var, "変数 " + var->name + " が見つかりません。") });
  }
}