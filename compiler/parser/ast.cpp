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
    return _tokenIndex + 1 < sequence.tokens.size();
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
    assert(false);
    return std::vector{ createASTError("構文解析中にファイルの終わりに達しました。" + addMessage, backToken().get()) };
  }
  Result<FunctionDefineNode*, std::vector<std::string>> ASTGenerator::fnDefGenerate()
  {
    assert(currentToken().kind == TokenKind::FnKeyword);
    auto begin = currentTokenIter();
    auto fnDef = std::make_unique<FunctionDefineNode>();
    std::vector<std::string> errors;
    if(!nextToken()) return error(createEOTError("関数名または(が必要です。"));
    if(currentToken().kind == TokenKind::Identify) {
      if(auto name = variableGenerate()) fnDef->name = name.get();
      else errors += name.err();
      if(!nextToken()) return error(errors + createEOTError("(が必要です。"));
    }
    if(auto args = argDefGenerate()) fnDef->args = std::move(args.get());
    else errors += args.err();
    auto token = nextToken();
    if(!token) return error(errors + createEOTError("関数の本文が必要です。"));
    if(token.get().kind == TokenKind::Colon) {
      if(!nextToken()) return error(createEOTError("戻り値の型名が必要です。"));
      if(auto retType = typeGenerate()) fnDef->retType = retType.get();
      else errors += retType.err();
      if(!nextToken()) return error(errors + createEOTError("関数の本文が必要です。"));
    }
    if(auto body = exprGenerate()) fnDef->body = body.get();
    else errors += body.err();
    if(errors.empty()) {
      fnDef->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(fnDef.release());
    }
    return error(errors);
  }
  Result<VariableDefineNode*, std::vector<std::string>> ASTGenerator::varDefGenerate()
  {
    assert(currentToken().kind == TokenKind::DefKeyword || currentToken().kind ==  TokenKind::MutKeyword);
    auto begin = currentTokenIter();
    auto varDef = std::make_unique<VariableDefineNode>();
    std::vector<std::string> errors;
    varDef->isMut = currentToken().kind == TokenKind::MutKeyword;
    if(!nextToken()) return error(createEOTError("変数名が必要です。"));
    if(auto name = variableGenerate()) varDef->name = name.get();
    else errors += name.err();
    if(auto token = nextToken()) {
      if(token.get().kind == TokenKind::Colon) {
        if(!nextToken()) return error(errors + createEOTError("型名が必要です。"));
        if(auto type = typeGenerate()) varDef->type = std::move(type.get());
        else errors += type.err();
        if (!(token = nextToken())) {
          varDef->tokens = std::vector(begin, currentTokenIter() + 1);
          return ok(varDef.release());
        }
      }
      if(token.get().kind == TokenKind::Asign) {
        if(!nextToken()) return error(errors + createEOTError("式が必要です。"));
        if(auto expr = exprGenerate()) varDef->init = expr.get();
        else errors += expr.err();
      }
    }
    if(errors.empty()) {
      varDef->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(varDef.release());
    }
    return error(errors);
  }
  Result<ClassDefineNode*, std::vector<std::string>> ASTGenerator::clsDefGenerate()
  {
    // TODO
    assert(false);
    return ok(nullptr);
  }
  Result<AliasDefineNode*, std::vector<std::string>> ASTGenerator::alsDefGenerate()
  {
    // TODO
    assert(false);
    return ok(nullptr);
  }
  Result<ImportNode*, std::vector<std::string>> ASTGenerator::importGenerate()
  {
    // TODO
    assert(false);
    return ok(nullptr);
  }
  Result<ExternNode*, std::vector<std::string>> ASTGenerator::externGenerate()
  {
    // TODO
    assert(false);
    return ok(nullptr);
  }
  Result<VariableNode*, std::vector<std::string>> ASTGenerator::variableGenerate()
  {
    auto begin = currentTokenIter();
    auto token = currentToken();
    if(token.kind == TokenKind::Identify) {
      auto name = token.value;
      std::vector<TypeNode*> generics;
      const auto backup = _tokenIndex;
      auto generic = nextToken();
      if(generic && generic.get().kind == TokenKind::LessThan) {
        nextToken();
        std::vector<std::unique_ptr<TypeNode>> ugenerics;
        while(true) {
          auto type = typeGenerate();
          if(!type) {
            _tokenIndex = backup;
            break;
          }
          ugenerics.push_back(std::unique_ptr<TypeNode>(type.get()));
          auto next = nextToken();
          if(!next || (next.get().kind != TokenKind::Comma && next.get().kind != TokenKind::GreaterThan)) {
            _tokenIndex = backup;
            break;
          }
          if(next.get().kind == TokenKind::Comma) {
            nextToken();
            continue;
          }
          else if(next.get().kind == TokenKind::GreaterThan) {
            generics.reserve(ugenerics.size());
            for(auto& gen : ugenerics) {
              generics.push_back(gen.release());
            }
            break;
          }
          else {
            assert(false);
          }
        }
      }
      else {
        _tokenIndex = backup;
      }
      auto var = new VariableNode(name, generics);
      var->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(var);
    }
    return error(std::vector{ createASTError("キーワードや記号は使用できません。", token) });
  }
  Result<TypeNode*, std::vector<std::string>> ASTGenerator::typeGenerate()
  {
    auto begin = currentTokenIter();
    std::unique_ptr<TypeNode> type;
    std::vector<std::string> errors;
    switch(currentToken().kind) {
      case TokenKind::I8Keyword:
        type = std::make_unique<I8TypeNode>();
        break;
      case TokenKind::I16Keyword:
        type = std::make_unique<I16TypeNode>();
        break;
      case TokenKind::I32Keyword:
        type = std::make_unique<I32TypeNode>();
        break;
      case TokenKind::I64Keyword:
        type = std::make_unique<I64TypeNode>();
        break;
      case TokenKind::U8Keyword:
        type = std::make_unique<U8TypeNode>();
        break;
      case TokenKind::U16Keyword:
        type = std::make_unique<U16TypeNode>();
        break;
      case TokenKind::U32Keyword:
        type = std::make_unique<U32TypeNode>();
        break;
      case TokenKind::U64Keyword:
        type = std::make_unique<U64TypeNode>();
        break;
      case TokenKind::F32Keyword:
        type = std::make_unique<F32TypeNode>();
        break;
      case TokenKind::F64Keyword:
        type = std::make_unique<F64TypeNode>();
        break;
      case TokenKind::CharKeyword:
        type = std::make_unique<CharTypeNode>();
        break;
      case TokenKind::BoolKeyword:
        type = std::make_unique<BoolTypeNode>();
        break;
      case TokenKind::VoidKeyword:
        type = std::make_unique<VoidTypeNode>();
        break;
      case TokenKind::PtrKeyword: {
        auto next = nextToken();
        if(!next) return error(createEOTError("<が必要です。"));
        if(next.get().kind != TokenKind::LessThan) return error(std::vector{ createASTError("<が必要です。", next.get()) });
        if(!nextToken()) return error(createEOTError("型名が必要です。"));
        auto base = typeGenerate();
        if(!base) return error(base.err());
        next = nextToken();
        if(!next) return error(createEOTError(">が必要です。"));
        if(next.get().kind != TokenKind::GreaterThan) return error(std::vector{ createASTError(">が必要です。", next.get()) });
        type = std::make_unique<PtrTypeNode>(base.get());
        break;
      }
      case TokenKind::FnKeyword: {
        auto next = nextToken();
        if(!next) return error(createEOTError("(が必要です。"));
        if(next.get().kind != TokenKind::LParen) {
          errors.push_back(createASTError("(が必要です。", next.get()));
          break;
        }
        next = nextToken();
        if(!next) return error(errors + createEOTError(")が必要です。"));
        std::vector<std::unique_ptr<TypeNode>> uargs;
        if(next.get().kind != TokenKind::RParen) {
          while(true) {
            auto arg = typeGenerate();
            if(!arg) {
              errors += arg.err();
              break;
            }
            uargs.push_back(std::unique_ptr<TypeNode>(arg.get()));
            next = nextToken();
            if(!next) return error(errors + createEOTError(")が必要です。"));
            if(next.get().kind == TokenKind::RParen) break;
            else if(next.get().kind == TokenKind::Comma) next = nextToken();
            else errors.push_back(createASTError(")が必要です。", next.get()));
          }
          if(!errors.empty()) break;
        }
        next = nextToken();
        if(!next) return error(errors + createEOTError(":が必要です。"));
        if(next.get().kind != TokenKind::Colon) {
          errors.push_back(createASTError(":が必要です。", next.get()));
          break;
        }
        if(!nextToken()) return error(errors + createEOTError("型名が必要です。"));
        auto retType = typeGenerate();
        if(!retType) {
          errors += retType.err();
          break;
        }
        std::vector<TypeNode*> args;
        args.reserve(uargs.size());
        for(auto& arg : uargs) {
          args.push_back(arg.release());
        }
        type = std::make_unique<FnTypeNode>(retType.get(), args);
        break;
      }
      case TokenKind::Identify: {
        std::vector<std::string> name{ currentToken().value };
        while(auto token = nextToken()) {
          if(token.get().kind == TokenKind::Scope) {
            token = nextToken();
            if(!token) return error(errors + createEOTError("型名が必要です。"));
            if(token.get().kind != TokenKind::Identify) {
              errors.push_back(createASTError("型名にキーワードや記号は使用できません。", token.get()));
              continue;
            }
            name.push_back(token.get().value);
          }
          else {
            backToken(true);
            break;
          }
        }
        type = std::make_unique<UserDefineTypeNode>(name);
        auto next = nextToken();
        if(!next || next.get().kind != TokenKind::LessThan) {
          backToken(true);
          break;
        }
        if(!nextToken()) return error(errors + createEOTError("型名が必要です。"));
        std::vector<std::unique_ptr<TypeNode>> types;
        while(true) {
          auto gen = typeGenerate();
          if(!gen) {
            errors += gen.err();
            break;
          }
          types.emplace_back(std::unique_ptr<TypeNode>(gen.get()));
          next = nextToken();
          if(!next) return error(errors + createEOTError(">が必要です。"));
          if(next.get().kind == TokenKind::GreaterThan) break;
          if(next.get().kind == TokenKind::Comma) {
            nextToken();
            continue;
          }
          errors.push_back(">が必要です。");
          break;
        }
        std::vector<TypeNode*> generics;
        generics.reserve(types.size());
        for(auto& t : types) {
          generics.push_back(t.release());
        }
        type = std::make_unique<GenericsTypeNode>(dynamic_cast<UserDefineTypeNode*>(type.release())->name, generics);
        break;
      }
      default:
        return error(std::vector{ createASTError("型名にキーワードや記号は使用できません。", currentToken()) });
    }
    auto token = nextToken();
    while(token && token.get().kind == TokenKind::LBracket) {
      token = nextToken();
      if(!token) return error(createEOTError("配列の大きさが必要です。"));
      if(token.get().kind != TokenKind::Integer) return error(std::vector{ createASTError("配列の大きさが必要です。", token.get()) });
      type = std::make_unique<ArrayTypeNode>(type.release(), std::stoull(token.get().value));
      token = nextToken();
      if(!token) return error(createEOTError("]が必要です。"));
      if(token.get().kind != TokenKind::RBracket) return error(std::vector{ createASTError("]が必要です。", token.get()) });
      token = nextToken();
    }
    backToken(true);
    if(errors.empty()) {
      type->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(type.release());
    }
    return error(errors);
  }
  Result<std::vector<ArgumentDefineNode*>, std::vector<std::string>> ASTGenerator::argDefGenerate()
  {
    assert(currentToken().kind == TokenKind::LParen);

    auto token = nextToken();
    if(!token) return error(createEOTError(")が必要です。"));
    if(token.get().kind == TokenKind::RParen) return ok(std::vector<ArgumentDefineNode*>());

    std::vector<std::unique_ptr<ArgumentDefineNode>> args;
    std::vector<std::string> errors;
    const auto argGen = [&]() -> Option<std::vector<std::string>> {
      auto begin = currentTokenIter();
      auto argDef = std::make_unique<ArgumentDefineNode>();
      if(token.get().kind == TokenKind::DefKeyword) {
        argDef->isMut = false;
        token = nextToken();
        if(!token) return some(createEOTError("引数名が必要です。"));
      }
      else if (token.get().kind == TokenKind::MutKeyword) {
        argDef->isMut = true;
        token = nextToken();
        if(!token) return some(createEOTError("引数名が必要です。"));
      }
      if(auto name = variableGenerate()) argDef->name = name.get();
      else errors += name.err();

      token = nextToken();
      if(!token) return some(createEOTError(")が必要です。"));
      if(token.get().kind == TokenKind::Colon) {
        token = nextToken();
        if(!token) return some(createEOTError("引数の型名が必要です。"));
        if(auto type = typeGenerate()) argDef->type = type.get();
        else errors += type.err();
        token = nextToken();
        if(!token) return some(createEOTError(")が必要です。"));
      }

      if(token.get().kind == TokenKind::Asign) {
        token = nextToken();
        if(!token) return some(createEOTError("式が必要です。"));
        if(auto expr = exprGenerate()) argDef->init = expr.get();
        else errors += expr.err();
      }

      argDef->tokens = std::vector(begin, currentTokenIter() + 1);
      args.emplace_back(std::move(argDef));
      return none;
    };

    if(auto res = argGen()) return error(errors + res.get());
    while(true) {
      if(currentToken().kind == TokenKind::Comma) {
        token = nextToken();
        if(!token) return error(errors + createEOTError("引数が必要です。"));
        if(auto res = argGen()) return error(errors + res.get());
      }
      else if(currentToken().kind == TokenKind::RParen) {
        break;
      }
      else {
        errors.push_back(createASTError(")が必要です。", token.get()));
        break;
      }
    }

    if(errors.empty()) {
      std::vector<ArgumentDefineNode*> result;
      for(auto& arg : args) {
        result.push_back(arg.release());
      }
      return ok(std::move(result));
    }
    return error(errors);
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::exprGenerate()
  {
    switch(currentToken().kind) {
      case TokenKind::FnKeyword: {
        auto fn = fnDefGenerate();
        if(!fn) return error(fn.err());
        return ok(fn.get());
      }
      case TokenKind::DefKeyword:
      case TokenKind::MutKeyword: {
        auto var = varDefGenerate();
        if(!var) return error(var.err());
        return ok(var.get());
      }
      default:
        return asignGenerate();
    }
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::asignGenerate()
  {
    auto begin = currentTokenIter();
    auto left = compGenerate();
    if(!left) return error(left.err());
    auto op = nextToken();
    if(!op || !includes({ TokenKind::Asign, TokenKind::AddAsign, TokenKind::SubAsign, TokenKind::MulAsign, TokenKind::DivAsign, TokenKind::ModAsign }, op.get().kind)) {
      backToken(true);
      left.get()->tokens = std::vector(begin, currentTokenIter() + 1);
      return ok(left.get());
    }

    if(!nextToken()) {
      delete left.get();
      return error(createEOTError("式が必要です。"));
    }
    auto right = exprGenerate();
    if(!right) {
      delete left.get();
      return error(right.err());
    }
    AsignNode* node = nullptr;
    switch(op.get().kind) {
      case TokenKind::Asign:
        node = new AsignNode(left.get(), right.get());
        break;
      case TokenKind::AddAsign:
        node = new AddAsignNode(left.get(), right.get());
        break;
      case TokenKind::SubAsign:
        node = new SubAsignNode(left.get(), right.get());
        break;
      case TokenKind::MulAsign:
        node = new MulAsignNode(left.get(), right.get());
        break;
      case TokenKind::DivAsign:
        node = new DivAsignNode(left.get(), right.get());
        break;
      case TokenKind::ModAsign:
        node = new ModAsignNode(left.get(), right.get());
        break;
      default:
        assert(false);
    }
    node->tokens = std::vector(begin, currentTokenIter() + 1);
    return ok(node);
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::compGenerate()
  {
    auto begin = currentTokenIter();
    auto left = termGenerate();
    if(!left) return error(left.err());

    auto result = left.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Equal, TokenKind::NotEqual, TokenKind::GreaterEqual, TokenKind::GreaterThan, TokenKind::LessEqual, TokenKind::LessThan }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(std::move(result));
      }
      if(!nextToken()) {
        delete result;
        return error(createEOTError("式が必要です。"));
      }
      auto right = termGenerate();
      if(!right) {
        delete result;
        return error(right.err());
      }
      switch(op.get().kind) {
        case TokenKind::Equal:
          result = new EqualNode(result, right.get());
          break;
        case TokenKind::NotEqual:
          result = new NotEqualNode(result, right.get());
          break;
        case TokenKind::GreaterEqual:
          result = new GreaterEqualNode(result, right.get());
          break;
        case TokenKind::GreaterThan:
          result = new GreaterThanNode(result, right.get());
          break;
        case TokenKind::LessEqual:
          result = new LessEqualNode(result, right.get());
          break;
        case TokenKind::LessThan:
          result = new LessThanNode(result, right.get());
          break;
        default:
          assert(false);
      }
    }
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::termGenerate()
  {
    auto begin = currentTokenIter();
    auto left = factorGenerate();
    if(!left) return error(left.err());

    auto result = left.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Plus, TokenKind::Minus }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(result);
      }
      if(!nextToken()) {
        delete result;
        return error(createEOTError("式が必要です。"));
      }
      auto right = factorGenerate();
      if(!right) {
        delete result;
        return error(right.err());
      }
      switch(op.get().kind) {
        case TokenKind::Plus:
          result = new AddNode(result, right.get());
          break;
        case TokenKind::Minus:
          result = new SubNode(result, right.get());
          break;
        default:
          assert(false);
      }
    }
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::factorGenerate()
  {
    auto begin = currentTokenIter();
    auto left = frontUnaryGenerate();
    if(!left) return error(left.err());

    auto result = left.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Asterisk, TokenKind::Slash, TokenKind::Percent }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(result);
      }
      if(!nextToken()) {
        delete result;
        return error(createEOTError("式が必要です。"));
      }
      auto right = frontUnaryGenerate();
      if(!right) {
        delete result;
        return error(right.err());
      }
      switch(op.get().kind) {
        case TokenKind::Asterisk:
          result = new MulNode(result, right.get());
          break;
        case TokenKind::Slash:
          result = new DivNode(result, right.get());
          break;
        case TokenKind::Percent:
          result = new ModNode(result, right.get());
          break;
        default:
          assert(false);
      }
    } 
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::frontUnaryGenerate()
  {
    if(includes({ TokenKind::Plus, TokenKind::Minus, TokenKind::Inc, TokenKind::Dec }, currentToken().kind)) {
      auto begin = currentTokenIter();
      const auto op = currentToken().kind;
      if(!nextToken()) return error(createEOTError("式が必要です。"));
      auto base = frontUnaryGenerate();
      if(!base) return error(base.err());
      FrontUnaryNode* node = nullptr;
      switch(op) {
        case TokenKind::Plus:
          node = new PlusNode(base.get());
          break;
        case TokenKind::Minus:
          node = new MinusNode(base.get());
          break;
        case TokenKind::Inc:
          node = new FrontIncrementNode(base.get());
          break;
        case TokenKind::Dec:
          node = new FrontDecrementNode(base.get());
          break;
        default:
          assert(false);
      }
      node->tokens = std::vector(begin, currentTokenIter() + 1);
    }
    return backUnaryGenerate();
  }
  Result<ExpressionNode*, std::vector<std::string>> ASTGenerator::backUnaryGenerate()
  {
    auto begin = currentTokenIter();
    auto base = primaryGenerate();
    if(!base) return error(base.err());
    auto result = base.get();
    while(true) {
      auto op = nextToken();
      if(!op || !includes({ TokenKind::Inc, TokenKind::Dec, TokenKind::Dot, TokenKind::LBracket, TokenKind::LParen }, op.get().kind)) {
        backToken(true);
        result->tokens = std::vector(begin, currentTokenIter() + 1);
        return ok(result);
      }
      switch(op.get().kind) {
        case TokenKind::Inc:
          result = new BackIncrementNode(result);
          break;
        case TokenKind::Dec:
          result = new BackDecrementNode(result);
          break;
        case TokenKind::Dot: {
          if(!nextToken()) {
            delete result;
            return error(createEOTError("メンバー名が必要です。"));
          }
          auto member = variableGenerate();
          if(!member) {
            delete result;
            return error(member.err());
          }
          result = new MemberAccessNode(result, member.get());
          break;
        }
        case TokenKind::LBracket: {
          if(!nextToken()) {
            delete result;
            return error(createEOTError("式が必要です。"));
          }
          auto suffix = exprGenerate();
          if(!suffix) {
            delete result;
            return error(suffix.err());
          }
          auto rb = nextToken();
          if(!rb) {
            delete result;
            return error(createEOTError("]が必要です。"));
          }
          if(rb.get().kind != TokenKind::RBracket) {
            delete result;
            return error(std::vector{ createASTError("]が必要です。", rb.get()) });
          }
          result = new ArrayAccessNode(result, suffix.get());
          break;
        }
        case TokenKind::LParen: {
          auto next = nextToken();
          if(!next) {
            delete result;
            return error(createEOTError(")が必要です。"));
          }
          std::vector<std::unique_ptr<ExpressionNode>> uargs;
          if(next.get().kind != TokenKind::RParen) {
            while(true) {
              auto arg = exprGenerate();
              if(!arg) {
                delete result;
                return error(arg.err());
              }
              uargs.emplace_back(std::unique_ptr<ExpressionNode>(arg.get()));
              next = nextToken();
              if(!next) {
                delete result;
                return error(createEOTError(")が必要です。"));
              }
              if(next.get().kind == TokenKind::RParen) break;
              if(next.get().kind == TokenKind::Comma) {
                if(!nextToken()) {
                  delete result;
                  return error(createEOTError("式が必要です。"));
                }
                continue;
              }
              delete result;
              return error(std::vector{createASTError(")が必要です。", next.get())});
            }
          }
          std::vector<ExpressionNode*> args;
          for(auto& arg : uargs) {
            args.push_back(arg.release());
          }
          result = new CallNode(result, std::move(args));
          break;
        }
        default:
          assert(false);
      }
    }
    return ok(result);
  }
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
              next = nextToken();
              if(!next) return error(createEOTError("式が必要です。"));
              if(includes({ TokenKind::Semicolon, TokenKind::RBrase }, next.get().kind)) {
                unodes.emplace_back(std::make_unique<ReturnNode>(nullptr));
              }
              else {
                auto expr = exprGenerate();
                if(expr) unodes.emplace_back(std::make_unique<ReturnNode>(expr.get()));
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
          break;
        }
        return error(errors);
      }
      default:
        assert(false);
    }
    node->tokens = std::vector(begin, currentTokenIter() + 1);
    return ok(node);
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
          break;
        }
        case TokenKind::TypeKeyword: {
          auto res = alsDefGenerate();
          break;
        }
        case TokenKind::ImportKeyword: {
          auto res = importGenerate();
          break;
        }
        case TokenKind::ExternKeyword: {
          auto res = externGenerate();
          break;
        }
        case TokenKind::Semicolon:
          break;
        default:
          errors.push_back(createASTError("予期しないトークン。宣言文を期待しました。", token.get()));
          // 同じエラーが連続するのを避けるため、次の宣言までスキップする
          while(hasNext()) {
            auto kind = nextToken().get().kind;
            if(
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