#include "compiler.h"

#include <iostream>
#include <sstream>
#include <filesystem>
#include <functional>

#include "dump.h"

namespace pick
{
    namespace
    {
        Result<ModuleNode, std::vector<std::string>> _createModuleTree(const std::string& dir)
        {
            using namespace std::filesystem;
            ModuleNode node;
            node.name = path(dir).filename().string();
            node.name = node.name.substr(0, node.name.find('.'));
            std::vector<std::string> errors;
            std::error_code err;
            for (directory_iterator itr(dir), end; itr != end && !err; itr.increment(err)) {
                const auto entry = *itr;
                if (is_directory(entry)) {
                    auto res = _createModuleTree(entry.path().string());
                    if (!res) {
                        errors += res.err();
                    }
                    else {
                        auto child = res.get();
                        node.children[child.name] = child;
                    }
                }
                else {
                    auto filename = entry.path().filename().string();
                    auto name = filename.substr(0, filename.find('.'));
                    node.modules[name].name = name;
                    node.modules[name].srcPath = entry.path().string();
                }
            }
            if (errors.empty()) return ok(node);
            else return error(errors);
        }
        Result<_, std::vector<std::string>> _tokenize(ModuleNode& node)
        {
            std::vector<std::string> errors;
            for (auto& mod : node.modules) {
                token::Tokenizer tokenizer(mod.second.srcPath);
                auto res = tokenizer.tokenize();
                if (!res) {
                    errors += res.err();
                }
                else {
                    mod.second.tokenSequence = res.get();
                }
            }
            for (auto& child : node.children) {
                _tokenize(child.second);
            }
            if (errors.empty()) return ok();
            else return error(errors);
        }
        Result<_, std::vector<std::string>> _parse(ModuleNode& node)
        {
            std::vector<std::string> errors;
            for (auto& mod : node.modules) {
                syntax::Parser parser(mod.second.tokenSequence);
                auto res = parser.parse();
                if (!res) {
                    errors += res.err();
                }
                else {
                    mod.second.syntaxTree = res.get();
                }
            }
            for (auto& child : node.children) {
                _parse(child.second);
            }
            if (errors.empty()) return ok();
            else return error(errors);
        }
    }
    Result<_, std::vector<std::string>> Compiler::createModuleTree()
    {
        if (!std::filesystem::is_directory(option.src)) {
            return error(std::vector{ (std::stringstream("エラー: --srcオプションはディレクトリを指定してください。指定されたオプション: ") << option.src).str() });
        }
        auto res = _createModuleTree(option.src);
        if (!res) return error(res.err());
        tree.nodes[option.name] = res.get();
        tree.nodes[option.name].name = option.name;
        return ok();
    }
    Result<_, std::vector<std::string>> Compiler::tokenize()
    {
        std::vector<std::string> errors;
        for (auto& node : tree.nodes) {
            auto res = _tokenize(node.second);
            if (!res) errors += res.err();
        }
        if (errors.empty()) return ok();
        else return error(errors);
    }
    Result<_, std::vector<std::string>> Compiler::parse()
    {
        std::vector<std::string> errors;
        for (auto& node : tree.nodes) {
            auto res = _parse(node.second);
            if (!res) errors += res.err();
        }
        if (errors.empty()) return ok();
        else return error(errors);
    }
    Result<_, std::vector<std::string>> Compiler::irGenerate()
    {
        ir::IR ir(tree);
        auto res = ir.generate();
        if (res) {
            this->ir = res.get();
            return ok();
        }
        else return error(res.err());
    }
    Result<_, std::vector<std::string>> Compiler::x86_64Compile()
    {
        x86_64::X86_64Compiler compiler(ir);
        auto res = compiler.compile();
        if (res) {
            x86_64 = res.get();
            return ok();
        }
        else return error(res.err());
    }
    Result<_, std::vector<std::string>> Compiler::exeLink()
    {
        exe::Linker linker(x86_64);
        auto res = linker.link("main", option.lib);
        if (!res) return error(res.err());
        std::filesystem::create_directories(option.out);
        res = linker.write(option.out + "\\" + option.name + ".exe");
        if (res) return ok();
        else return error(res.err());
    }
    Compiler::Compiler(const CompilerOption& option) : option(option)
    {
        assert(!option.src.empty());
        assert(!option.out.empty());
        assert(!option.name.empty());
    }
    int Compiler::compile()
    {
#define MAYBE_ERROR(res) if(!res) { std::cerr << "\033[31m"; for(const auto& err : res.err()) std::cerr << err << std::endl; std::cerr << "\033[m"; return -1; }
        auto res = createModuleTree();
        MAYBE_ERROR(res);
        res = tokenize();
        MAYBE_ERROR(res);
        res = parse();
        MAYBE_ERROR(res);
        if (option.compilerDebugMode) dump::moduleDump(tree);
        // TODO: load libs
        res = irGenerate();
        if (option.compilerDebugMode) dump::irDump(ir);
        MAYBE_ERROR(res);
        res = x86_64Compile();
        MAYBE_ERROR(res);
        res = exeLink();
        MAYBE_ERROR(res);
#undef MAYBE_ERROR
        return 0;
    }
}
