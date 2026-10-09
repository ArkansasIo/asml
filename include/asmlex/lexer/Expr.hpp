// ============================================================================
//  ASMLEX - include/asmlex/lexer/Expr.hpp
//  Recursive-descent constant expression evaluator used by the assembler for
//  operands such as `label + 4`, `(count * 8) - 2`, `1 << shift`.
// ============================================================================
#pragma once

#include <asmlex/core/Diagnostics.hpp>
#include <asmlex/core/Types.hpp>
#include <asmlex/lexer/Lexer.hpp>
#include <asmlex/lexer/Token.hpp>

#include <functional>
#include <vector>

namespace asmlex::lexer
{

  /// Result of evaluating a constant expression.
  struct ExprValue
  {
    i64 value = 0;
    bool is_float = false;
    bool unresolved = false; ///< depends on a symbol that is not yet known
    String symbol;           ///< the unresolved symbol, when !unresolved is false

    [[nodiscard]] bool ok() const noexcept { return !unresolved; }
  };

  /// Callback used to resolve a symbol name to a value.  Returning false marks
  /// the expression as unresolved (a forward reference).
  using SymbolResolver = std::function<bool(StringView name, i64 &out)>;

  class ExpressionParser
  {
  public:
    ExpressionParser(const TokenStream &stream, DiagnosticSink &sink)
        : stream_(stream), sink_(sink) {}

    /// Attach a resolver for symbolic names.
    void set_resolver(SymbolResolver resolver) { resolver_ = std::move(resolver); }

    /// Parse a full expression starting at token index `pos`; advances `pos`
    /// past the last consumed token.
    [[nodiscard]] ExprValue parse(usize &pos);

    /// Convenience: parse the expression contained in `stream` from index 0.
    [[nodiscard]] ExprValue parse_all()
    {
      usize pos = 0;
      return parse(pos);
    }

  private:
    [[nodiscard]] ExprValue parse_or(usize &pos);
    [[nodiscard]] ExprValue parse_and(usize &pos);
    [[nodiscard]] ExprValue parse_bitor(usize &pos);
    [[nodiscard]] ExprValue parse_bitxor(usize &pos);
    [[nodiscard]] ExprValue parse_bitand(usize &pos);
    [[nodiscard]] ExprValue parse_shift(usize &pos);
    [[nodiscard]] ExprValue parse_additive(usize &pos);
    [[nodiscard]] ExprValue parse_term(usize &pos);
    [[nodiscard]] ExprValue parse_unary(usize &pos);
    [[nodiscard]] ExprValue parse_primary(usize &pos);

    [[nodiscard]] const Token &peek(usize pos) const noexcept { return stream_.at(pos); }
    [[nodiscard]] bool accept_op(usize &pos, StringView op);
    [[nodiscard]] bool accept_kw(usize &pos, StringView kw);
    [[nodiscard]] ExprValue error(StringView text, usize pos);

    /// Fold a binary op over two values, propagating unresolved operands.
    [[nodiscard]] ExprValue fold(const ExprValue &a, const ExprValue &b, StringView op, u32 line,
                                 u32 column);

    const TokenStream &stream_;
    DiagnosticSink &sink_;
    SymbolResolver resolver_{};
  };

  /// Evaluate a raw expression string (convenience wrapper that lexes first).
  [[nodiscard]] ExprValue evaluate_expression(StringView text, DiagnosticSink &sink,
                                              const SymbolResolver &resolver = {},
                                              Dialect dialect = Dialect::Nasm);

  /// Parse an integer literal token ("0x7B", "0b101", "0o17", "123").
  [[nodiscard]] bool parse_integer_literal(StringView text, i64 &out);

} // namespace asmlex::lexer
