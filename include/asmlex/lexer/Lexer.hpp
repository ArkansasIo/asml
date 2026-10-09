// ============================================================================
//  ASMLEX - include/asmlex/lexer/Lexer.hpp
//  Turns raw assembly / hex source text into a TokenStream.
// ============================================================================
#pragma once

#include <asmlex/core/Diagnostics.hpp>
#include <asmlex/lexer/Token.hpp>

#include <string_view>

namespace asmlex::lexer
{

  /// Dialects understood by the lexer.  NASM is the reference syntax; the other
  /// modes relax or tighten a few rules (comment characters, byte runs, ...).
  enum class Dialect
  {
    Nasm,
    Gas,
    Masm,
    Hex ///< pure "48 8B C1" byte listing
  };

  [[nodiscard]] inline StringView to_string(Dialect d) noexcept
  {
    switch (d)
    {
    case Dialect::Nasm:
      return "nasm";
    case Dialect::Gas:
      return "gas";
    case Dialect::Masm:
      return "masm";
    case Dialect::Hex:
      return "hex";
    }
    return "nasm";
  }

  /// Options controlling a single lex run.
  struct LexOptions
  {
    Dialect dialect = Dialect::Nasm;
    String file = "<memory>";
    bool keep_comments = true;
    bool allow_hex_bytes = true;
  };

  class Lexer
  {
  public:
    Lexer(LexOptions options, DiagnosticSink &sink)
        : options_(std::move(options)), sink_(sink) {}

    /// Lex `source` into a fresh token stream (always ends with EndOfFile).
    [[nodiscard]] TokenStream run(StringView source);

    /// Detect the dialect that best fits `source` (used when opening a file).
    [[nodiscard]] static Dialect detect_dialect(StringView source, StringView extension = {});

    /// Names of registers known to the built-in x86-64 profile, used for
    /// syntax highlighting and operand classification.
    [[nodiscard]] static bool is_register(StringView name);
    [[nodiscard]] static bool is_directive(StringView name);

  private:
    [[nodiscard]] bool match_hex_bytes(StringView source, usize i) const;
    [[nodiscard]] Token lex_number(StringView source, usize &i, SourcePos pos);
    [[nodiscard]] Token lex_identifier(StringView source, usize &i, SourcePos pos);
    [[nodiscard]] Token lex_string(StringView source, usize &i, SourcePos pos);
    [[nodiscard]] Token lex_directive(StringView source, usize &i, SourcePos pos);

    LexOptions options_;
    DiagnosticSink &sink_;
  };

} // namespace asmlex::lexer
