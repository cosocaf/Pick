#include "ast.h"

#include "pickc/config.h"
#include "utils/vector_utils.h"

namespace pickc::parser
{
  ASTGenerator::ASTGenerator(const TokenSequence& sequence) : sequence(sequence), _tokenIndex(-1) {}
  Option<Token> ASTGenerator::nextToken(bool index)
  {
    if(_tokenIndex + 1 < sequence.tokens.size()) {
      if(includes({ TokenKind::LineComment }, sequence.tokens[_tokenIndex + 1].kind)) {
        ++_tokenIndex;
        return nextToken(index);
      }
      if(index) return some(sequence.tokens[++_tokenIndex]);
      return some(sequence.tokens[_tokenIndex + 1]);
    }
    if(index) ++_tokenIndex;
    return none;
  }
  Option<Token> ASTGenerator::backToken(bool index)
  {
    if(_tokenIndex > 0) {
      if(includes({ TokenKind::LineComment }, sequence.tokens[_tokenIndex - 1].kind)) {
        --_tokenIndex;
        return backToken(index);
      }
      if(index) return some(sequence.tokens[--_tokenIndex]);
      return some(sequence.tokens[_tokenIndex - 1]);
    }
    if(index) --_tokenIndex;
    return none;
  }
  Token ASTGenerator::currentToken()
  {
    return sequence.tokens[_tokenIndex];
  }
  bool ASTGenerator::hasNext()
  {
    auto res = nextToken();
    backToken(true);
    return res;
  }
  std::vector<pickc::parser::Token>::iterator ASTGenerator::currentTokenIter()
  {
    return sequence.tokens.begin() + _tokenIndex;
  }
  std::string ASTGenerator::createASTError(const std::string& message, const Token& token)
  {
    std::string res;
    res += "構文エラー: ";
    res += message;
    res += "\nat ";
    res += sequence.file;
    res += ' ';
    res += std::to_string(token.line);
    res += "行目 ";
    res += std::to_string(token.letter);
    res += "文字目\n";
    const auto line = token.line - 1;
    const auto letter = token.letter - 1;
    if(line > 0) res += sequence.toOutputString(line - 1);
    res += sequence.toOutputString(line);
    auto digit = 3 + letter;
    {
      auto n = token.line;
      while(n != 0) {
          n /= 10;
          ++digit;
      }
    }
    res += CONSOLE_FG_RED;
    res.append(digit, ' ');
    res.append(token.value.size(), '^');
    res += '\n';
    if(line + 1 < sequence.rawFileData.size()) sequence.toOutputString(line + 1);
    return res;
  }
  std::vector<std::string> ASTGenerator::createEOTError(const std::string& addMessage)
  {
    return std::vector{ createASTError("構文解析中にファイルの終わりに達しました。" + addMessage, backToken().get()) };
  }
  Result<RootNode, std::vector<std::string>> ASTGenerator::generate()
  {
    RootNode rootNode;
    std::vector<std::string> errors;
    while(hasNext()) {
      auto token = nextToken();
      bool isPub = token.get().kind == TokenKind::PubKeyword;
      if(token.get().kind == TokenKind::PubKeyword || token.get().kind == TokenKind::PriKeyword) {
        token = nextToken();
        if(!token) {
          return error(createEOTError("fn, def, mut, class, typeが必要です。"));
        }
      }
      switch(token.get().kind) {
        case TokenKind::FnKeyword:
          if(auto fnDefRes = fnDefGenerate()) {
            auto fnDef = fnDefRes.get();
            fnDef->isPub = isPub;
            rootNode.nodes.push_back(fnDef);
          }
          else errors += fnDefRes.err();
          break;
        case TokenKind::DefKeyword:
        case TokenKind::MutKeyword:
          if(auto varDefRes = varDefGenerate()) {
            auto varDef = varDefRes.get();
            varDef->isPub = isPub;
            rootNode.nodes.push_back(varDef);
          }
          else errors += varDefRes.err();
          break;
        case TokenKind::ClassKeyword: {
          auto res = clsDefGenerate();
          assert(false);
          break;
        }
        case TokenKind::TypeKeyword: {
          auto res = alsDefGenerate();
          assert(false);
          break;
        }
        case TokenKind::ImportKeyword:
          if(auto impRes = importGenerate()) rootNode.nodes.push_back(impRes.get());
          else errors += impRes.err();
          break;
        case TokenKind::ExternKeyword:
          if(auto extRes = externGenerate()) {
            auto ext = extRes.get();
            ext->isPub = isPub;
            rootNode.nodes.push_back(ext);
          }
          else errors += extRes.err();
          break;
        case TokenKind::Semicolon:
          break;
        default:
          errors.push_back(createASTError("予期しないトークン。宣言文を期待しました。", token.get()));
          // 同じエラーが連続するのを避けるため、次の宣言までスキップする
          while(hasNext()) {
            auto kind = nextToken().get().kind;
            if(
              kind == TokenKind::PubKeyword ||
              kind == TokenKind::PriKeyword ||
              kind == TokenKind::FnKeyword ||
              kind == TokenKind::DefKeyword ||
              kind == TokenKind::MutKeyword ||
              kind == TokenKind::ClassKeyword ||
              kind == TokenKind::TypeKeyword ||
              kind == TokenKind::ImportKeyword ||
              kind == TokenKind::ExternKeyword
            ) {
              backToken(true);
              break;
            }
          }
      }
    }
    rootNode.tokens = std::move(sequence.tokens);
    if(errors.empty()) return ok(std::move(rootNode));
    else return error(errors);
  }
}