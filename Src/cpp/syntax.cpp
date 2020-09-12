#include "syntax.h"

#include <sstream>

namespace pick::syntax
{
    Result<token::Token, std::vector<std::string>> Parser::next()
    {
        using namespace token;
        if (++currentSequence < sequence.tokens.size()) {
            if (sequence.tokens[currentSequence].kind == TokenKind::LineComment) return next();
            return ok(sequence.tokens[currentSequence]);
        }
        else return error(createErrorMessage("構文の解析中にファイルの終わりに達しました。"));
    }
    Result<token::Token, std::vector<std::string>> Parser::current()
    {
        using namespace token;
        if (currentSequence < sequence.tokens.size()) {
            if (sequence.tokens[currentSequence].kind == TokenKind::LineComment) return next();
            return ok(sequence.tokens[currentSequence]);
        }
        else return error(createErrorMessage("構文の解析中にファイルの終わりに達しました。"));
    }
    void Parser::back()
    {
        --currentSequence;
    }
    std::vector<std::string> Parser::createErrorMessage(const std::string& message, const token::Token* source)
    {
        std::stringstream ss("構文エラー: ");
        ss << message << "\n    at " << sequence.fileName;
        if (source) {
            ss << " " << source->line << "行目, " << source->letter << "文字目";
        }
        return std::vector{ ss.str() };
    }
    void Parser::skip(size_t size)
    {
        currentSequence += size;
    }
    bool Parser::eos()
    {
        return currentSequence + 1 >= sequence.tokens.size();
    }
    bool Parser::isBackUnaryOperator(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::IncOperator:
        case TokenKind::DecOperator:
        case TokenKind::Dot:
        case TokenKind::ScopeOperator:
        case TokenKind::LBracket:
        case TokenKind::LParen:
            return true;
        default:
            return false;
        }
    }
    bool Parser::isFrontUnaryOperator(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::PlusSign:
        case TokenKind::MinusSign:
        case TokenKind::IncOperator:
        case TokenKind::DecOperator:
            return true;
        default:
            return false;
        }
    }
    bool Parser::isFactorOperator(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::Asterisk:
        case TokenKind::Slash:
        case TokenKind::Percent:
            return true;
        default:
            return false;
        }
    }
    bool Parser::isTermOperator(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::PlusSign:
        case TokenKind::MinusSign:
            return true;
        default:
            return false;
        }
    }
    bool Parser::isCompOperator(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::EqualOperator:
        case TokenKind::NotEqualOperator:
        case TokenKind::GreaterEqualOperator:
        case TokenKind::GreaterThanOperator:
        case TokenKind::LessEqualOperator:
        case TokenKind::LessThanOperator:
            return true;
        default:
            return false;
        }
    }
    bool Parser::isAsignOperator(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::AsignOperator:
        case TokenKind::AddAsignOperator:
        case TokenKind::SubAsignOperator:
        case TokenKind::MulAsignOperator:
        case TokenKind::DivAsignOperator:
        case TokenKind::ModAsignOperator:
            return true;
        default:
            return false;
        }
    }
    bool Parser::isLiteral(token::TokenKind kind)
    {
        using namespace token;
        switch (kind) {
        case TokenKind::IntegerLiteral:
        case TokenKind::I8Literal:
        case TokenKind::I16Literal:
        case TokenKind::I32Literal:
        case TokenKind::I64Literal:
        case TokenKind::U8Literal:
        case TokenKind::U16Literal:
        case TokenKind::U32Literal:
        case TokenKind::U64Literal:
        case TokenKind::FloatLiteral:
        case TokenKind::F32Literal:
        case TokenKind::F64Literal:
        case TokenKind::True:
        case TokenKind::False:
        case TokenKind::CharLiteral:
        case TokenKind::StringLiteral:
        case TokenKind::LBracket:
        case TokenKind::This:
            return true;
        default:
            return false;
        }
    }
    Result<Node*, std::vector<std::string>> Parser::backUnary()
    {
        using namespace token;
        auto pri = primary();
        if (!pri) return error(pri.err());
        auto op = next();
        if (!op || !isBackUnaryOperator(op.get().kind)) {
            back();
            return ok(pri.get());
        }
        auto result = new Node;
        result->kind = NodeKind::BackUnary;
        result->back = new BackUnary{};
        result->back->expr = pri.get();
        switch (op.get().kind) {
        case TokenKind::IncOperator:
            result->back->op = BackUnaryOperator::Inc;
            break;
        case TokenKind::DecOperator:
            result->back->op = BackUnaryOperator::Dec;
            break;
        case TokenKind::Dot:
            result->back->op = BackUnaryOperator::Member;
            {
                skip();
                auto mem = variable();
                if (!mem) {
                    delete result;
                    return error(mem.err());
                }
                result->back->member = mem.get();
            }
            break;
        case TokenKind::LBracket:
            result->back->op = BackUnaryOperator::Array;
            {
                skip();
                auto suffix = expr();
                if (!suffix) return error(suffix.err());
                result->back->suffix = suffix.get();
                auto rbracket = next();
                if (!rbracket) return error(rbracket.err());
                if (rbracket.get().kind != TokenKind::RBracket) return error(createErrorMessage("]が必要です。", &rbracket.get()));
            }
            break;
        case TokenKind::LParen:
            result->back->op = BackUnaryOperator::Call;
            {
                auto res = next();
                if (!res) return error(res.err());
                auto args = new Arguments{};
                if (res.get().kind != TokenKind::RParen) {
                    while (true) {
                        auto arg = expr();
                        if (!arg) {
                            delete args;
                            return error(arg.err());
                        }
                        args->args.push_back(arg.get());
                        res = next();
                        if (!res) {
                            delete args;
                            return error(res.err());
                        }
                        if (res.get().kind == TokenKind::RParen) break;
                        else if (res.get().kind == TokenKind::Comma) skip();
                        else {
                            delete args;
                            return error(createErrorMessage("')'か','が必要です。", &res.get()));
                        }
                    }
                }
                result->back->args = args;
            }
            break;
        default:
            assert(false);
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::frontUnary()
    {
        using namespace token;
        auto unary = current();
        if (!unary) return error(unary.err());
        
        if (isFrontUnaryOperator(unary.get().kind)) {
            auto result = new Node{};
            result->kind = NodeKind::FrontUnary;
            result->front = new FrontUnary{};
            switch (unary.get().kind) {
            case TokenKind::PlusSign: result->front->op = FrontUnaryOperator::Plus; break;
            case TokenKind::MinusSign: result->front->op = FrontUnaryOperator::Minus; break;
            case TokenKind::IncOperator: result->front->op = FrontUnaryOperator::Inc; break;
            case TokenKind::DecOperator: result->front->op = FrontUnaryOperator::Dec; break;
            default: assert(false);
            }
            skip();

            auto back = backUnary();
            if (!back) {
                delete result;
                return error(back.err());
            }
            result->front->expr = back.get();
            return ok(result);
        }
        else {
            return backUnary();
        }
    }
    Result<Node*, std::vector<std::string>> Parser::factor()
    {
        using namespace token;
        auto left = frontUnary();
        if (!left) return error(left.err());
        auto op = next();
        if (!op || !isFactorOperator(op.get().kind)) {
            back();
            return ok(left.get());
        }

        auto result = new Node{};
        result->kind = NodeKind::Factor;
        result->factor = new Factor{};
        result->factor->left = left.get();
        while (true) {
            skip();
            auto right = frontUnary();
            if (!right) return error(right.err());
            result->factor->right = right.get();
            switch (op.get().kind) {
            case TokenKind::Asterisk: result->factor->op = FactorOperator::Mul; break;
            case TokenKind::Slash: result->factor->op = FactorOperator::Div; break;
            case TokenKind::Percent: result->factor->op = FactorOperator::Mod; break;
            default: assert(false);
            }
            op = next();
            if (!op || !isFactorOperator(op.get().kind)) {
                back();
                break;
            }
            auto node = new Node{};
            node->kind = NodeKind::Factor;
            node->factor = new Factor{};
            node->factor->left = result;
            result = node;
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::term()
    {
        using namespace token;
        auto left = factor();
        if (!left) return error(left.err());
        auto op = next();
        if (!op || !isTermOperator(op.get().kind)) {
            back();
            return ok(left.get());
        }
        auto result = new Node{};
        result->kind = NodeKind::Term;
        result->term = new Term{};
        result->term->left = left.get();
        while (true) {
            skip();
            auto right = factor();
            if (!right) return error(right.err());
            result->term->right = right.get();
            switch (op.get().kind) {
            case TokenKind::PlusSign: result->term->op = TermOperator::Add; break;
            case TokenKind::MinusSign: result->term->op = TermOperator::Sub; break;
            default: assert(false);
            }
            op = next();
            if (!op || !isTermOperator(op.get().kind)) {
                back();
                break;
            }
            auto node = new Node{};
            node->kind = NodeKind::Term;
            node->term = new Term{};
            node->term->left = result;
            result = node;
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::comp()
    {
        using namespace token;
        auto left = term();
        if (!left) return error(left.err());
        auto op = next();
        if (!op || !isCompOperator(op.get().kind)) {
            back();
            return ok(left.get());
        }

        auto result = new Node{};
        result->kind = NodeKind::Comparison;
        result->comp = new Comparison{};
        result->comp->left = left.get();
        while (true) {
            skip();
            auto right = term();
            if (!right) return error(right.err());
            result->comp->right = right.get();
            switch (op.get().kind) {
            case TokenKind::EqualOperator: result->comp->op = ComparisonOperator::Equal; break;
            case TokenKind::NotEqualOperator: result->comp->op = ComparisonOperator::NotEqual; break;
            case TokenKind::GreaterEqualOperator: result->comp->op = ComparisonOperator::GreaterEqual; break;
            case TokenKind::GreaterThanOperator: result->comp->op = ComparisonOperator::GreaterThan; break;
            case TokenKind::LessEqualOperator: result->comp->op = ComparisonOperator::LessEqual; break;
            case TokenKind::LessThanOperator: result->comp->op = ComparisonOperator::LessThan; break;
            default: assert(false);
            }
            op = next();
            if (!op || !isCompOperator(op.get().kind)) {
                back();
                break;
            }
            auto node = new Node{};
            node->kind = NodeKind::Comparison;
            node->comp = new Comparison{};
            node->comp->left = result;
            result = node;
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::asign()
    {
        using namespace token;
        auto left = comp();
        if (!left) return error(left.err());
        auto op = next();
        if (!op || !isAsignOperator(op.get().kind)) {
            back();
            return ok(left.get());
        }
        skip();
        auto right = asign();
        if (!right) return error(right.err());
        auto result = new Node{};
        result->kind = NodeKind::Asign;
        result->asign = new Asign{};
        result->asign->left = left.get();
        result->asign->right = right.get();
        switch (op.get().kind) {
        case TokenKind::AsignOperator: result->asign->op = AsignOperator::Asign; break;
        case TokenKind::AddAsignOperator: result->asign->op = AsignOperator::Add; break;
        case TokenKind::SubAsignOperator: result->asign->op = AsignOperator::Sub; break;
        case TokenKind::MulAsignOperator: result->asign->op = AsignOperator::Mul; break;
        case TokenKind::DivAsignOperator: result->asign->op = AsignOperator::Div; break;
        case TokenKind::ModAsignOperator: result->asign->op = AsignOperator::Mod; break;
        default: assert(false);
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::literal()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        auto token = res.get();
        auto result = new Node{};
        result->kind = NodeKind::Literal;
        result->literal = new Literal{};
        switch (token.kind) {
        case TokenKind::IntegerLiteral:
            result->literal->type = LiteralType::Integer;
            result->literal->integer = std::stoi(token.value);
            break;
        case TokenKind::I8Literal:
            result->literal->type = LiteralType::I8;
            result->literal->i8 = int8_t(std::stoi(token.value.substr(0, token.value.size() - 2)));
            break;
        case TokenKind::I16Literal:
            result->literal->type = LiteralType::I16;
            result->literal->i16 = int16_t(std::stoi(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::I32Literal:
            result->literal->type = LiteralType::I32;
            result->literal->i32 = int32_t(std::stoi(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::I64Literal:
            result->literal->type = LiteralType::I64;
            result->literal->i64 = int64_t(std::stoll(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::U8Literal:
            result->literal->type = LiteralType::U8;
            result->literal->u8 = uint8_t(std::stoi(token.value.substr(0, token.value.size() - 2)));
            break;
        case TokenKind::U16Literal:
            result->literal->type = LiteralType::U16;
            result->literal->u16 = uint16_t(std::stoi(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::U32Literal:
            result->literal->type = LiteralType::U32;
            result->literal->u32 = uint32_t(std::stoul(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::U64Literal:
            result->literal->type = LiteralType::U64;
            result->literal->u64 = uint64_t(std::stoull(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::FloatLiteral:
            result->literal->type = LiteralType::Float;
            result->literal->f = double(std::stod(token.value.substr(0, token.value.size())));
            break;
        case TokenKind::F32Literal:
            result->literal->type = LiteralType::F32;
            result->literal->f32 = float(std::stof(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::F64Literal:
            result->literal->type = LiteralType::F64;
            result->literal->f64 = double(std::stod(token.value.substr(0, token.value.size() - 3)));
            break;
        case TokenKind::True:
            result->literal->type = LiteralType::True;
            break;
        case TokenKind::False:
            result->literal->type = LiteralType::False;
            break;
        case TokenKind::CharLiteral:
            assert(token.value.size() == 1);
            result->literal->type = LiteralType::Char;
            result->literal->c = token.value[0];
            break;
        case TokenKind::StringLiteral:
            result->literal->type = LiteralType::String;
            result->literal->string = new std::string(token.value + '\0');
            break;
        case TokenKind::LBracket:
        {
            result->literal->type = LiteralType::Array;
            result->literal->array = new ArrayLiteral{};
            auto res = next();
            if (!res) return error(res.err());
            if (res.get().kind == TokenKind::RBracket) break;
            while (true) {
                auto elem = expr();
                if (!elem) return error(elem.err());
                result->literal->array->elems += elem.get();
                res = next();
                if (!res) return error(res.err());
                if (res.get().kind == TokenKind::RBracket) break;
                else if (res.get().kind == TokenKind::Comma) {
                    skip();
                }
                else {
                    return error(createErrorMessage(",か]が必要です。", &res.get()));
                }
            }
            break;
        }
        case TokenKind::This:
            result->literal->type = LiteralType::This;
            break;
        default:
            assert(false);
        }
        return ok(result);
    }
    Result<Variable*, std::vector<std::string>> Parser::variable()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::Identify) {
            return error(createErrorMessage("変数名に記号やキーワードは使用できません。", &res.get()));
        }
        auto result = new Variable{};
        result->name = res.get().value;
        auto scope = next();
        if (!scope || scope.get().kind != TokenKind::ScopeOperator) {
            back();
            return ok(result);
        }
        skip();
        auto right = variable();
        if (!right) return error(right.err());
        result->scope = right.get();
        return ok(result);
    }
    Result<Type*, std::vector<std::string>> Parser::type()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        auto result = new Type{};
        switch (res.get().kind) {
        case TokenKind::I8Keyword: result->type = TypeType::I8; break;
        case TokenKind::I16Keyword: result->type = TypeType::I16; break;
        case TokenKind::I32Keyword: result->type = TypeType::I32; break;
        case TokenKind::I64Keyword: result->type = TypeType::I64; break;
        case TokenKind::U8Keyword: result->type = TypeType::U8; break;
        case TokenKind::U16Keyword: result->type = TypeType::U16; break;
        case TokenKind::U32Keyword: result->type = TypeType::U32; break;
        case TokenKind::U64Keyword: result->type = TypeType::U64; break;
        case TokenKind::CharKeyword: result->type = TypeType::Char; break;
        case TokenKind::BoolKeyword: result->type = TypeType::Bool; break;
        case TokenKind::VoidKeyword: result->type = TypeType::Void; break;
        case TokenKind::Identify:
        {
            result->type = TypeType::Variable;
            auto var = variable();
            if (!var) return error(var.err());
            result->var = var.get();
            break;
        }
        default:
            delete result;
            return error(createErrorMessage("不正な型名です。", &res.get()));
        }
        res = next();
        if (!res || res.get().kind != TokenKind::LBracket) {
            back();
        }
        else {
            res = next();
            if (!res) {
                delete result;
                return error(res.err());
            }
            else if (res.get().kind != TokenKind::RBracket) {
                delete result;
                return error(createErrorMessage("]が必要です。", &res.get()));
            }
            auto array = new Type{};
            array->type = TypeType::Array;
            array->elem = result;
            result = array;
        }
        return ok(result);
    }
    Result<Return*, std::vector<std::string>> Parser::ret()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::ReturnKeyword) {
            return error(createErrorMessage("returnが必要です。", &res.get()));
        }
        skip();
        auto ret = expr();
        if (!ret) {
            return error(ret.err());
        }
        auto result = new Return{};
        result->val = ret.get();
        return ok(result);
    }
    Result<VariableDefine*, std::vector<std::string>> Parser::varDef()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::DefKeyword && res.get().kind != TokenKind::MutKeyword) {
            return error(createErrorMessage("変数の定義にはdefかmutが必要です。", &res.get()));
        }
        skip();
        auto var = variable();
        if (!var) return error(var.err());
        auto result = new VariableDefine{};
        result->var = var.get();
        auto colon = next();
        if (!colon) {
            back();
            return ok(result);
        }
        if (colon.get().kind == TokenKind::Colon) {
            skip();
            auto t = type();
            if (!t) {
                delete result;
                return error(t.err());
            }
            result->type = t.get();
        }
        else {
            back();
        }
        auto asign = next();
        if (!asign || asign.get().kind != TokenKind::AsignOperator) {
            back();
            return ok(result);
        }
        skip();
        auto init = expr();
        if (!init) {
            delete result;
            return error(init.err());
        }
        result->init = init.get();
        return ok(result);
    }
    Result<ArgumentDefine*, std::vector<std::string>> Parser::argDef()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind == TokenKind::DefKeyword || res.get().kind == TokenKind::MutKeyword) {
            skip();
        }
        auto var = variable();
        if (!var) return error(var.err());
        auto result = new ArgumentDefine{};
        result->var = var.get();
        auto colon = next();
        if (!colon) return ok(result);
        if (colon.get().kind == TokenKind::Colon) {
            skip();
            auto t = type();
            if (!t) {
                delete result;
                return error(t.err());
            }
            result->type = t.get();
        }
        else {
            back();
        }
        auto asign = next();
        if (!asign || asign.get().kind != TokenKind::AsignOperator) {
            back();
            return ok(result);
        }
        skip();
        auto init = expr();
        if (!init) {
            delete result;
            return error(init.err());
        }
        result->init = init.get();
        return ok(result);
    }
    Result<ExternDeclare*, std::vector<std::string>> Parser::externDec()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::ExternKeyword) {
            return error(createErrorMessage("externが必要です。", &res.get()));
        }
        res = next();
        if (!res) return error(res.err());
        auto ext = new ExternDeclare{};
        if (res.get().kind != TokenKind::Identify) {
            delete ext;
            return error(createErrorMessage("記号やキーワードは使用できません。", &res.get()));
        }
        auto var = variable();
        if (!var) {
            delete ext;
            return error(res.err());
        }
        ext->name = var.get();
        res = next();
        if (!res) {
            delete ext;
            return error(res.err());
        }
        if (res.get().kind != TokenKind::LParen) {
            delete ext;
            return error(createErrorMessage("'('が必要です。", &res.get()));
        }
        res = next();
        if (res.get().kind != TokenKind::RParen) {
            while (true) {
                auto arg = argDef();
                if (!arg) {
                    delete ext;
                    return error(arg.err());
                }
                ext->args.push_back(arg.get());
                res = next();
                if (res.get().kind == TokenKind::RParen) break;
                else if (res.get().kind == TokenKind::Comma) skip();
                else {
                    delete ext;
                    return error(createErrorMessage("')'か','が必要です。", &res.get()));
                }
            }
        }
        auto colon = next();
        if (!colon) {
            delete ext;
            return error(colon.err());
        }
        if (colon.get().kind != TokenKind::Colon) {
            delete ext;
            return error(createErrorMessage(":が必要です。", &colon.get()));
        }
        skip();
        auto t = type();
        if (!t) {
            delete ext;
            return error(t.err());
        }
        ext->retType = t.get();
        return ok(ext);
    }
    Result<ImportModule*, std::vector<std::string>> Parser::importMod()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::ImportKeyword) {
            return error(createErrorMessage("importが必要です。", &res.get()));
        }
        skip();
        auto var = variable();
        if (!var) return error(res.err());

        auto imp = new ImportModule{};
        imp->name = var.get();

        return ok(imp);
    }
    Result<FunctionDefine*, std::vector<std::string>> Parser::fnDef()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::FnKeyword) {
            return error(createErrorMessage("関数の定義にはfnが必要です。", &res.get()));
        }
        res = next();
        if (!res) return error(res.err());
        auto fn = new FunctionDefine{};
        if (res.get().kind == TokenKind::Identify) {
            auto var = variable();
            if (!var) {
                delete fn;
                return error(res.err());
            }
            fn->name = var.get();
            res = next();
        }
        if (res.get().kind != TokenKind::LParen) {
            delete fn;
            return error(createErrorMessage("'('が必要です。", &res.get()));
        }
        res = next();
        if (res.get().kind != TokenKind::RParen) {
            while (true) {
                auto arg = argDef();
                if (!arg) {
                    delete fn;
                    return error(arg.err());
                }
                fn->args.push_back(arg.get());
                res = next();
                if (res.get().kind == TokenKind::RParen) break;
                else if (res.get().kind == TokenKind::Comma) skip();
                else {
                    delete fn;
                    return error(createErrorMessage("')'か','が必要です。", &res.get()));
                }
            }
        }
        auto colon = next();
        if (!colon) {
            delete fn;
            return error(colon.err());
        }
        if (colon.get().kind == TokenKind::Colon) {
            skip();
            auto t = type();
            if (!t) {
                delete fn;
                return error(t.err());
            }
            fn->retType = t.get();
        }
        else {
            back();
        }
        skip();
        auto exp = expr();
        if (!exp) return error(exp.err());
        fn->expr = exp.get();
        return ok(fn);
    }
    Result<ClassDefine*, std::vector<std::string>> Parser::clsDef()
    {
        assert(false);
        return error("");
    }
    Result<AliasDefine*, std::vector<std::string>> Parser::alsDef()
    {
        assert(false);
        return error("");
    }
    Result<Node*, std::vector<std::string>> Parser::block()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        if (res.get().kind != TokenKind::LBrace) {
            return error(createErrorMessage("'{'が必要です。", &res.get()));
        }
        skip();
        auto result = new Node{};
        result->kind = NodeKind::Block;
        result->block = new Block{};
        while (!eos()) {
            auto exp = expr();
            if (!exp) {
                delete result;
                return error(exp.err());
            }
            result->block->exprs.push_back(exp.get());
            while (!eos()) {
                auto semicolon = next();
                if (!semicolon) break;
                if (semicolon.get().kind != TokenKind::Semicolon) {
                    back();
                    break;
                }
            }
            res = next();
            if (!res) {
                delete result;
                return error(res.err());
            }
            if (res.get().kind == TokenKind::RBrace) break;
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::expr()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        auto result = new Node{};
        auto token = res.get();
        if (token.kind == TokenKind::DefKeyword || token.kind == TokenKind::MutKeyword) {
            result->kind = NodeKind::VariableDefine;
            auto var = varDef();
            if (!var) {
                delete result;
                return error(var.err());
            }
            result->varDef = var.get();
        }
        else if (token.kind == TokenKind::FnKeyword) {
            result->kind = NodeKind::FunctionDefine;
            auto fn = fnDef();
            if (!fn) {
                delete result;
                return error(fn.err());
            }
            result->fnDef = fn.get();
        }
        else if (token.kind == TokenKind::ExternKeyword) {
            result->kind = NodeKind::ExternDeclare;
            auto ext = externDec();
            if (!ext) {
                delete result;
                return error(ext.err());
            }
            result->extDec = ext.get();
        }
        else if (token.kind == TokenKind::ImportKeyword) {
            result->kind = NodeKind::ImportModule;
            auto imp = importMod();
            if (!imp) {
                delete result;
                return error(imp.err());
            }
            result->impMod = imp.get();
        }
        else if (token.kind == TokenKind::ClassKeyword) {
            result->kind = NodeKind::ClassDefine;
            auto cls = clsDef();
            if (!cls) {
                delete result;
                return error(cls.err());
            }
            result->clsDef = cls.get();
        }
        else if (token.kind == TokenKind::TypeKeyword) {
            result->kind = NodeKind::AliasDefine;
            auto als = alsDef();
            if (!als) {
                delete result;
                return error(als.err());
            }
            result->alsDef = als.get();
        }
        else if (token.kind == TokenKind::ReturnKeyword) {
            result->kind = NodeKind::Return;
            auto r = ret();
            if (!r) {
                delete result;
                return error(r.err());
            }
            result->ret = r.get();
        }
        else {
            delete result;
            return asign();
        }
        return ok(result);
    }
    Result<Node*, std::vector<std::string>> Parser::primary()
    {
        using namespace token;
        auto res = current();
        if (!res) return error(res.err());
        auto result = new Node{};
        result->kind = NodeKind::Primary;
        result->primary = new Primary{};
        auto token = res.get();
        if (isLiteral(token.kind)) {
            auto lit = literal();
            if (!lit) {
                delete result;
                return error(lit.err());
            }
            assert(lit.get()->kind == NodeKind::Literal);
            result->primary->type = PrimaryType::Literal;
            result->primary->literal = lit.get()->literal;
        }
        else if (token.kind == TokenKind::Identify) {
            auto var = variable();
            if (!var) {
                delete result;
                return error(var.err());
            }
            result->primary->type = PrimaryType::Variable;
            result->primary->var = var.get();
        }
        else if (token.kind == TokenKind::LParen) {
            skip();
            auto exp = expr();
            if (!exp) {
                delete result;
                return error(exp.err());
            }
            auto rparen = next();
            if (!rparen) {
                delete result;
                return error(res.err());
            }
            if (rparen.get().kind != TokenKind::RParen) {
                delete result;
                return error(createErrorMessage("')'が必要です。", &rparen.get()));
            }
            result->primary->type = PrimaryType::Expression;
            result->primary->expr = exp.get();
        }
        else if (token.kind == TokenKind::LBrace) {
            auto blk = block();
            if (!blk) {
                delete result;
                return error(blk.err());
            }
            assert(blk.get()->kind == NodeKind::Block);
            result->primary->type = PrimaryType::Block;
            result->primary->block = blk.get()->block;
        }
        else if (token.kind == TokenKind::IfKeyword) {
            auto lparen = next();
            if (!lparen) {
                delete result;
                return error(lparen.err());
            }
            if (lparen.get().kind != TokenKind::LParen) {
                delete result;
                return error(createErrorMessage("(が必要です。", &lparen.get()));
            }
            skip();
            auto cond = expr();
            if (!cond) {
                delete result;
                return error(cond.err());
            }
            auto rparen = next();
            if (!rparen) {
                delete result;
                return error(rparen.err());
            }
            if (rparen.get().kind != TokenKind::RParen) {
                delete result;
                return error(createErrorMessage(")が必要です。", &rparen.get()));
            }
            skip();
            auto ifExpr = expr();
            if (!ifExpr) {
                delete result;
                return error(ifExpr.err());
            }

            result->primary->type = PrimaryType::IfElse;
            result->primary->ifElse = new IfElse{};
            result->primary->ifElse->cond = cond.get();
            result->primary->ifElse->ifExpr = ifExpr.get();

            auto elseKeyword = next();
            if (!elseKeyword || elseKeyword.get().kind != TokenKind::ElseKeyword) {
                back();
            }
            else {
                skip();
                auto elseExpr = expr();
                if (!elseExpr) {
                    delete result;
                    return error(elseExpr.err());
                }
                result->primary->ifElse->elseExpr = elseExpr.get();
            }
        }
        else if (token.kind == TokenKind::WhileKeyword) {
            auto lparen = next();
            if (!lparen) {
                delete result;
                return error(lparen.err());
            }
            if (lparen.get().kind != TokenKind::LParen) {
                delete result;
                return error(createErrorMessage("(が必要です。", &lparen.get()));
            }
            skip();
            auto cond = expr();
            if (!cond) {
                delete result;
                return error(cond.err());
            }
            auto rparen = next();
            if (!rparen) {
                delete result;
                return error(rparen.err());
            }
            if (rparen.get().kind != TokenKind::RParen) {
                delete result;
                return error(createErrorMessage(")が必要です。", &rparen.get()));
            }
            skip();

            auto exp = expr();
            if (!exp) {
                delete result;
                return error(exp.err());
            }

            result->primary->type = PrimaryType::While;
            result->primary->whi = new While{};
            result->primary->whi->cond = cond.get();
            result->primary->whi->expr = exp.get();
        }
        else {
            return error(createErrorMessage("不正なトークンです。", &res.get()));
        }
        return ok(result);
    }
    Parser::Parser(const token::TokenSequence& sequence) : sequence(sequence), done(false), currentSequence(0) {}
    Result<SyntaxTree, std::vector<std::string>> Parser::parse()
    {
        assert(done == false);
        using namespace token;
        while (!eos()) {
            auto res = expr();
            if (!res) errors += res.err();
            else tree.exprs.push_back(res.get());
            while (!eos()) {
                auto semicolon = next();
                if (!semicolon || semicolon.get().kind != TokenKind::Semicolon) {
                    break;
                }
            }
        }
        done = true;
        if (errors.empty()) return ok(tree);
        else return error(errors);
    }
}