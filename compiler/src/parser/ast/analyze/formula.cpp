#include "../generator.h"

#include "output_buffer.h"

namespace pickc::parser {
  std::optional<FormulaNode> ASTGenerator::analyzeFormula(ASTNode parentNode) {
    auto token = cur().value();
    switch(token.kind) {
      case TokenKind::LBrace:
        return analyzeBlock(parentNode);
      case TokenKind::Integer:
        return analyzeImmediate(parentNode);
      default:
        outputBuffer.emplace<CompileErrorMessage>(sequence.filename, token.line, token.letter, token.value.size(), "式ではありません。");
        return std::nullopt;
    }
  }
}