#include "token.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>

namespace pick::token
{
    void Tokenizer::token()
    {
        if (buf.empty()) return;

        if (buf == "fn") sequence.tokens += Token(TokenKind::FnKeyword, line, letter, buf);
        else if (buf == "def") sequence.tokens += Token(TokenKind::DefKeyword, line, letter, buf);
        else if (buf == "mut") sequence.tokens += Token(TokenKind::MutKeyword, line, letter, buf);
        else if (buf == "extern") sequence.tokens += Token(TokenKind::ExternKeyword, line, letter, buf);
        else if (buf == "import") sequence.tokens += Token(TokenKind::ImportKeyword, line, letter, buf);
        else if (buf == "return") sequence.tokens += Token(TokenKind::ReturnKeyword, line, letter, buf);
        else if (buf == "if") sequence.tokens += Token(TokenKind::IfKeyword, line, letter, buf);
        else if (buf == "else") sequence.tokens += Token(TokenKind::ElseKeyword, line, letter, buf);
        else if (buf == "for") sequence.tokens += Token(TokenKind::ForKeyword, line, letter, buf);
        else if (buf == "while") sequence.tokens += Token(TokenKind::WhileKeyword, line, letter, buf);
        else if (buf == "loop") sequence.tokens += Token(TokenKind::LoopKeyword, line, letter, buf);
        else if (buf == "switch") sequence.tokens += Token(TokenKind::SwitchKeyword, line, letter, buf);
        else if (buf == "pub") sequence.tokens += Token(TokenKind::PubKeyword, line, letter, buf);
        else if (buf == "pri") sequence.tokens += Token(TokenKind::PriKeyword, line, letter, buf);
        else if (buf == "class") sequence.tokens += Token(TokenKind::ClassKeyword, line, letter, buf);
        else if (buf == "construct") sequence.tokens += Token(TokenKind::ConstructKeyword, line, letter, buf);
        else if (buf == "destruct") sequence.tokens += Token(TokenKind::DestructKeyword, line, letter, buf);
        else if (buf == "type") sequence.tokens += Token(TokenKind::TypeKeyword, line, letter, buf);
        else if (buf == "i8") sequence.tokens += Token(TokenKind::I8Keyword, line, letter, buf);
        else if (buf == "i16") sequence.tokens += Token(TokenKind::I16Keyword, line, letter, buf);
        else if (buf == "i32") sequence.tokens += Token(TokenKind::I32Keyword, line, letter, buf);
        else if (buf == "i64") sequence.tokens += Token(TokenKind::I64Keyword, line, letter, buf);
        else if (buf == "u8") sequence.tokens += Token(TokenKind::U8Keyword, line, letter, buf);
        else if (buf == "u16") sequence.tokens += Token(TokenKind::U16Keyword, line, letter, buf);
        else if (buf == "u32") sequence.tokens += Token(TokenKind::U32Keyword, line, letter, buf);
        else if (buf == "u64") sequence.tokens += Token(TokenKind::U64Keyword, line, letter, buf);
        else if (buf == "f32") sequence.tokens += Token(TokenKind::F32Keyword, line, letter, buf);
        else if (buf == "f64") sequence.tokens += Token(TokenKind::F64Keyword, line, letter, buf);
        else if (buf == "void") sequence.tokens += Token(TokenKind::VoidKeyword, line, letter, buf);
        else if (buf == "bool") sequence.tokens += Token(TokenKind::BoolKeyword, line, letter, buf);
        else if (buf == "char") sequence.tokens += Token(TokenKind::CharKeyword, line, letter, buf);
        else if (buf == "true") sequence.tokens += Token(TokenKind::True, line, letter, buf);
        else if (buf == "false") sequence.tokens += Token(TokenKind::False, line, letter, buf);
        else if (buf == "this") sequence.tokens += Token(TokenKind::This, line, letter, buf);
        else {
            std::smatch match;
            if (std::regex_match(buf, match, std::regex(R"(^\d+$)"))) sequence.tokens += Token(TokenKind::IntegerLiteral, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+i8$)"))) sequence.tokens += Token(TokenKind::I8Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+i16$)"))) sequence.tokens += Token(TokenKind::I16Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+i32$)"))) sequence.tokens += Token(TokenKind::I32Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+i64$)"))) sequence.tokens += Token(TokenKind::I64Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+u8$)"))) sequence.tokens += Token(TokenKind::U8Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+u16$)"))) sequence.tokens += Token(TokenKind::U16Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+u32$)"))) sequence.tokens += Token(TokenKind::U32Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+u64$)"))) sequence.tokens += Token(TokenKind::U64Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+(e[\-+]?\d+)?$)"))) sequence.tokens += Token(TokenKind::FloatLiteral, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+(e[\-+]?\d+)?f32$)"))) sequence.tokens += Token(TokenKind::F32Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+(e[\-+]?\d+)?f64$)"))) sequence.tokens += Token(TokenKind::F64Literal, line, letter, buf);
            else if (std::regex_match(buf, match, std::regex(R"(^[^\s!-?\[-\^`\{-~][^\s!-/:-?\[-\^`\{-~]*$)"))) sequence.tokens += Token(TokenKind::Identify, line, letter, buf);
            else {
                std::stringstream ss;
                ss << "トークンエラー: 不明なトークン " << buf << "\n at    " << sequence.fileName << " " << line << "行目, " << letter << "文字目";
                pushError(errors, ss.str());
            }
        }

        buf.clear();
    }
    Tokenizer::Tokenizer(const std::string& path) : done(false)
    {
        sequence.fileName = path;
    }
    Result<TokenSequence, std::vector<std::string>> Tokenizer::tokenize()
    {
        assert(done == false);
        std::ifstream stream(sequence.fileName);
        if (!stream) {
            return error(std::vector{ (std::stringstream("エラー: ファイルを開けませんでした。ファイル名: ") << sequence.fileName).str() });
        }
        line = 1;
        std::string str;
        while (std::getline(stream, str)) {
            letter = 1;
            for (auto l = str.size(); letter <= l; ++letter) {
                const char c = str[letter - 1];
                if (isspace(c)) {
                    token();
                    continue;
                }
                switch (c) {
                case '(': token(); sequence.tokens += Token(TokenKind::LParen, line, letter, "("); break;
                case ')': token(); sequence.tokens += Token(TokenKind::RParen, line, letter, ")"); break;
                case '{': token(); sequence.tokens += Token(TokenKind::LBrace, line, letter, "{"); break;
                case '}': token(); sequence.tokens += Token(TokenKind::RBrace, line, letter, "}"); break;
                case '[': token(); sequence.tokens += Token(TokenKind::LBracket, line, letter, "["); break;
                case ']': token(); sequence.tokens += Token(TokenKind::RBracket, line, letter, "]"); break;
                case ',': token(); sequence.tokens += Token(TokenKind::Comma, line, letter, ","); break;
                case ';': token(); sequence.tokens += Token(TokenKind::Semicolon, line, letter, ";"); break;
                case '.':
                    if (letter + 1 <= l && str[letter] == '.') {
                        token();
                        sequence.tokens += Token(TokenKind::RangeOperator, line, letter, "..");
                        ++letter;
                    }
                    else if (std::all_of(buf.begin(), buf.end(), [](char c) {return std::isdigit(c); })) {
                        buf += str[letter - 1];
                    }
                    else {
                        token();
                        sequence.tokens += Token(TokenKind::Dot, line, letter, ".");
                    }
                    break;
                case ':':
                    token();
                    if (letter + 1 <= l && str[letter] == ':') {
                        sequence.tokens += Token(TokenKind::ScopeOperator, line, letter, "::");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::Colon, line, letter, ":");
                    }
                    break;
                case '+':
                {
                    std::smatch match;
                    if (letter + 1 <= l && str[letter] == '+') {
                        token();
                        sequence.tokens += Token(TokenKind::IncOperator, line, letter, "++");
                        ++letter;
                    }
                    else if (letter + 1 <= l && str[letter] == '=') {
                        token();
                        sequence.tokens += Token(TokenKind::AddAsignOperator, line, letter, "+=");
                        ++letter;
                    }
                    else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+e$)"))) {
                        buf += str[letter - 1];
                    }
                    else {
                        token();
                        sequence.tokens += Token(TokenKind::PlusSign, line, letter, "+");
                    }
                    break;
                }
                case '-':
                {
                    std::smatch match;
                    if (letter + 1 <= l && str[letter] == '-') {
                        token();
                        sequence.tokens += Token(TokenKind::DecOperator, line, letter, "--");
                        ++letter;
                    }
                    else if (letter + 1 <= l && str[letter] == '=') {
                        token();
                        sequence.tokens += Token(TokenKind::SubAsignOperator, line, letter, "-=");
                        ++letter;
                    }
                    else if (std::regex_match(buf, match, std::regex(R"(^\d+?\.?\d+e$)"))) {
                        buf += str[letter - 1];
                    }
                    else {
                        token();
                        sequence.tokens += Token(TokenKind::MinusSign, line, letter, "-");
                    }
                    break;
                }
                case '*':
                    token();
                    if (letter + 1 <= l && str[letter] == '*') {
                        sequence.tokens += Token(TokenKind::ExpOperator, line, letter, "**");
                        ++letter;
                    }
                    else if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::MulAsignOperator, line, letter, "*=");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::Asterisk, line, letter, "*");
                    }
                    break;
                case '/':
                    token();
                    if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::DivAsignOperator, line, letter, "/=");
                        ++letter;
                    }
                    else if (letter + 1 <= l && str[letter] == '/') {
                        std::string comment = str.substr(letter + 2);
                        sequence.tokens += Token(TokenKind::LineComment, line, letter, comment);
                        letter = l;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::Slash, line, letter, "/");
                    }
                    break;
                case '%':
                    token();
                    if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::ModAsignOperator, line, letter, "%=");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::Percent, line, letter, "%");
                    }
                    break;
                case '=':
                    token();
                    if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::EqualOperator, line, letter, "==");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::AsignOperator, line, letter, "=");
                    }
                    break;
                case '!':
                    token();
                    if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::NotEqualOperator, line, letter, "!=");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::NotOperator, line, letter, "!");
                    }
                    break;
                case '>':
                    token();
                    if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::GreaterEqualOperator, line, letter, ">=");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::GreaterThanOperator, line, letter, ">");
                    }
                    break;
                case '<':
                    token();
                    if (letter + 1 <= l && str[letter] == '=') {
                        sequence.tokens += Token(TokenKind::LessEqualOperator, line, letter, "<=");
                        ++letter;
                    }
                    else {
                        sequence.tokens += Token(TokenKind::LessThanOperator, line, letter, "<");
                    }
                    break;
                case '\'':
                    token();
                    if (letter + 2 <= l && str[letter + 1] == '\'') {
                        sequence.tokens += Token(TokenKind::CharLiteral, line, letter, std::string(1, str[letter]));
                        letter += 2;
                    }
                    else {
                        std::stringstream ss;
                        ss << "トークンエラー: 文字リテラルの終わり引用符がありませんでした。\n    at " << sequence.fileName << " " << line << "行目, " << letter << "文字目";
                        pushError(errors, ss.str());
                    }
                    break;
                case '"':
                {
                    token();
                    std::string s;
                    bool eos = false;
                    auto let = letter;
                    for (++letter; letter <= l; ++letter) {
                        if (str[letter - 1] == '"') {
                            eos = true;
                            break;
                        }
                        else if (str[letter - 1] == '\\') {
                            if (++letter <= l) {
                                char escape;
                                switch (str[letter - 1]) {
                                case 'a': escape = '\a'; break;
                                case 'b': escape = '\b'; break;
                                case 'n': escape = '\n'; break;
                                case 'r': escape = '\r'; break;
                                case 'f': escape = '\f'; break;
                                case 't': escape = '\t'; break;
                                case 'v': escape = '\v'; break;
                                case '\\': escape = '\\'; break;
                                case '\'': escape = '\''; break;
                                case '"': escape = '"'; break;
                                case '0': escape = '\0'; break;
                                default:
                                    std::stringstream ss;
                                    ss << "トークンエラー: 不正なエスケープ文字です。\n    at " << sequence.fileName << " " << line << "行目, " << letter << "文字目";
                                    pushError(errors, ss.str());
                                }
                                s += escape;
                            }
                            else {
                                std::stringstream ss;
                                ss << "トークンエラー: 文字列リテラルの終わり引用符がありませんでした。\n    at " << sequence.fileName << " " << line << "行目, " << letter << "文字目";
                                pushError(errors, ss.str());
                            }
                        }
                        else {
                            s += str[letter - 1];
                        }
                    }
                    if (eos) {
                        sequence.tokens += Token(TokenKind::StringLiteral, line, let, s);
                    }
                    else {
                        std::stringstream ss;
                        ss << "トークンエラー: 文字列リテラルの終わり引用符がありませんでした。\n    at " << sequence.fileName << " " << line << "行目, " << letter << "文字目";
                        pushError(errors, ss.str());
                    }
                    break;
                }
                case '`':
                    token();
                    // tempLiteral = true;
                    break;
                case ' ':
                    token();
                    break;
                default:
                    buf += c;
                }
            }
            token();
            ++line;
        }
        token();
        done = true;
        if (errors.empty()) return ok(sequence);
        else return error(errors);
    }
    Token::Token(TokenKind kind, size_t line, size_t letter, const std::string& value) : kind(kind), line(line), letter(letter), value(value) {}
}
