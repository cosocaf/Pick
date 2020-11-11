#include "ast.h"

#include "utils/vector_utils.h"

namespace pickc::parser
{
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::primaryGenerate()
  {
    auto begin = currentTokenIter();
    const auto kind = currentToken().kind;
    const auto value = currentToken().value;
    ExpressionNode* node = nullptr;
    switch(kind) {
      case TokenKind::Integer:
        node = new IntegerLiteral(std::stoi(value));
        break;
      case TokenKind::I8:
        node = new I8Literal(static_cast<int8_t>(std::stoi(value.substr(0, value.size() - 2))));
        break;
      case TokenKind::I16:
        node = new I16Literal(static_cast<int16_t>(std::stoi(value.substr(0, value.size() - 3))));
        break;
      case TokenKind::I32:
        node = new I32Literal(std::stoi(value.substr(0, value.size() - 3)));
        break;
      case TokenKind::I64:
        node = new I64Literal(std::stoll(value.substr(0, value.size() - 3)));
        break;
      case TokenKind::U8:
        node = new U8Literal(static_cast<uint8_t>(std::stoul(value.substr(0, value.size() - 2))));
        break;
      case TokenKind::U16:
        node = new U16Literal(static_cast<uint16_t>(std::stoul(value.substr(0, value.size() - 3))));
        break;
      case TokenKind::U32:
        node = new U32Literal(std::stoul(value.substr(0, value.size() - 3)));
        break;
      case TokenKind::U64:
        node = new U64Literal(std::stoull(value.substr(0, value.size() - 3)));
        break;
      case TokenKind::Float:
        node = new FloatLiteral(std::stod(value));
        break;
      case TokenKind::F32:
        node = new F32Literal(std::stof(value));
        break;
      case TokenKind::F64:
        node = new F64Literal(std::stod(value));
        break;
      case TokenKind::Bool:
        assert(value == "true" || value == "false");
        node = new BoolLiteral(value == "true");
        break;
      case TokenKind::Null:
        node = new NullLiteral();
        break;
      case TokenKind::Char:
        node = new CharLiteral(value[0]);
        break;
      case TokenKind::String:
        node = new StringLiteral(value);
        break;
      case TokenKind::LBracket: {
        // TODO: 配列リテラル
        assert(false);
      }
      case TokenKind::Identify: {
        auto var = variableGenerate();
        if(!var) return error(var.err());
        auto result = var.get();
        while(true) {
          auto scope = nextToken();
          if(!scope || scope.get().kind != TokenKind::Scope) {
            backToken(true);
            node = result;
            break;
          }
          if(!nextToken()) return error(createEOTError("変数名が必要です。"));
          var = variableGenerate();
          if(!var) return error(var.err());
          result = new ScopedVariableNode(var.get()->name, var.get()->generics, result);
        }
        break;
      }
      case TokenKind::LParen: {
        if(!nextToken()) return error(createEOTError("式が必要です。"));
        auto expr = exprGenerate();
        if(!expr) return error(expr.err());
        auto rp = nextToken();
        if(!rp) {
          delete expr.get();
          return error(createEOTError(")が必要です。"));
        }
        if(rp.get().kind != TokenKind::RParen) {
          delete expr.get();
          return error(std::vector{ createASTError(")が必要です。", rp.get()) });
        }
        node = expr.get();
        break;
      }
      case TokenKind::LBrace: {
        std::vector<std::unique_ptr<Node>> unodes;
        std::vector<std::string> errors;
        while(true) {
          auto next = nextToken();
          if(!next) return error(errors + createEOTError("}が必要です。"));
          if(next.get().kind == TokenKind::RBrase) break;
          switch(next.get().kind) {
            case TokenKind::DefKeyword:
            case TokenKind::MutKeyword: {
              if(auto varDef = varDefGenerate()) unodes.emplace_back(std::move(varDef.get()));
              else errors += varDef.err();
              break;
            }
            case TokenKind::FnKeyword: {
              if(auto fnDef = fnDefGenerate()) unodes.emplace_back(std::move(fnDef.get()));
              else errors += fnDef.err();
              break;
            }
            case TokenKind::ReturnKeyword: {
              auto now = currentTokenIter();
              next = nextToken();
              if(!next) return error(createEOTError("式が必要です。"));
              if(includes({ TokenKind::Semicolon, TokenKind::RBrase }, next.get().kind)) {
                auto ret = std::make_unique<ReturnNode>(nullptr);
                ret->tokens = std::vector(now, currentTokenIter() + 1);
                unodes.emplace_back(std::move(ret));
              }
              else {
                auto expr = exprGenerate();
                auto ret = std::make_unique<ReturnNode>(expr.get());
                ret->tokens = std::vector(now, currentTokenIter() + 1);
                if(expr) unodes.emplace_back(std::move(ret));
                else errors += expr.err();
              }
              break;
            }
            case TokenKind::Semicolon:
              // skip token
              break;
            default: {
              auto expr = exprGenerate();
              if(expr) unodes.emplace_back(std::unique_ptr<Node>(expr.get()));
              else errors += expr.err();
            }
          }
        }
        if(errors.empty()) {
          auto block = new BlockNode();
          block->nodes.reserve(unodes.size());
          for(auto& unode : unodes) {
            block->nodes.push_back(unode.release());
          }
          node = block;
        }
        else {
          return error(errors);
        }
        break;
      }
      case TokenKind::IfKeyword: {
        std::vector<std::string> errors;
        if(!nextToken()) return error(createEOTError("(が必要です。"));
        if(currentToken().kind != TokenKind::LParen) errors.push_back("(が必要です。");
        if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
        auto comp = exprGenerate();
        if(!comp) errors += comp.err();
        if(!nextToken()) return error(errors + createEOTError(")が必要です。"));
        if(currentToken().kind != TokenKind::RParen) errors.push_back(")が必要です。");
        if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
        auto thenExpr = exprGenerate();
        if(!thenExpr) errors += thenExpr.err();
        ExpressionNode* elseExpr = nullptr;
        if(auto els = nextToken()) {
          if(els.get().kind == TokenKind::ElseKeyword) {
            if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
            if(auto res = exprGenerate()) elseExpr = res.get();
            else errors += res.err();
          }
        }
        if(errors.empty()) {
          auto ifNode = new IfNode();
          ifNode->comp = comp.get();
          ifNode->thenExpr = thenExpr.get();
          ifNode->elseExpr = elseExpr;
          node = ifNode;
        }
        else {
          return error(errors);
        }
        break;
      }
      case TokenKind::WhileKeyword: {
        std::vector<std::string> errors;
        if(!nextToken()) return error(createEOTError("(が必要です。"));
        if(currentToken().kind != TokenKind::LParen) errors.push_back("(が必要です。");
        if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
        auto comp = exprGenerate();
        if(!comp) errors += comp.err();
        if(!nextToken()) return error(errors + createEOTError(")が必要です。"));
        if(currentToken().kind != TokenKind::RParen) errors.push_back(")が必要です。");
        if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
        auto body = exprGenerate();
        if(!body) errors += body.err();
        auto whileNode = new WhileNode();
        whileNode->comp = comp.get();
        whileNode->body = body.get();
        node = whileNode;
        break;
      }
      default:
        assert(false);
        return error(std::vector{ createASTError("予期しないトークンです。", currentToken()) });
    }
    node->tokens = std::vector(begin, currentTokenIter() + 1);
    return ok(node);
  }
}