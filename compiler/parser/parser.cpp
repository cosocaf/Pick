#include "parser.h"

#include <iostream>
#include <filesystem>

#include "utils/result.h"
#include "utils/string_utils.h"
#include "utils/vector_utils.h"

#include "ast.h"

namespace pickc::parser
{
  namespace
  {
    Option<std::vector<std::string>> createModuleTree(const std::string& dir, ModuleTree* parent)
    {
      std::error_code err;
      std::vector<std::string> errors;
      for(std::filesystem::directory_iterator itr(dir), end; itr != end && !err; itr.increment(err)) {
        const auto entry = *itr;
        auto filename = entry.path().filename().string();
        if(std::filesystem::is_directory(entry)) {
          auto tree = new ModuleTree();
          tree->name = parent->name + "::" + filename;
          tree->parent = parent;
          parent->submodules[filename] = tree;
          if(auto res = createModuleTree(entry.path().string(), tree)) errors += res.get();
        }
        else if(endsWith(filename, ".pick")) {
          auto name = filename.substr(0, filename.rfind('.'));
          ModuleTree* tree;
          if(name == "index") tree = parent;
          else {
            tree = new ModuleTree();
            tree->name = parent->name + "::" + name;
            tree->parent = parent;
            parent->submodules[name] = tree;
          }
          if(auto res = Tokenizer(std::filesystem::absolute(entry).string()).tokenize()) {
            tree->sequence = std::move(res.get());
          }
          else {
            errors += res.err();
            continue;
          }
          if(auto res = ASTGenerator(tree->sequence).generate()) {
            tree->ast = std::move(res.get());
          }
          else {
            errors += res.err();
            continue;
          }
        }
      }
      if(errors.empty()) return none;
      return some(errors);
    }
  }
  Parser::Parser() {}
  Result<ModuleTree*, std::vector<std::string>> Parser::parse(const CompilerOption& option)
  {
    auto root = new ModuleTree();
    root->name = option.projectName;
    if(auto res = createModuleTree(option.srcDir, root)) return error(res.get());
    if(option.compilerDebug) {
      std::cout << "AST Dump" << std::endl;
      dump(root);
    }
    return ok(root);
  }
  void Parser::dump(const ModuleTree* tree) {
    if(!tree->sequence.file.empty()) {
      std::cout << "Dump of file " << tree->sequence.file << std::endl;
      tree->ast.dump();
      std::cout << std::endl;
    }
    for(auto& sub : tree->submodules) dump(sub.second);
  }
}