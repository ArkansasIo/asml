// ============================================================================
//  ASMLEX - include/asmlex/lexer/TokenPrinter.hpp
//  Rendering helpers that turn a TokenStream back into text: pretty columnar
//  listings for the lexer pane, compact one-line summaries and round-trip
//  reconstruction of the original source.
// ============================================================================
#pragma once

#include <asmlex/lexer/Lexer.hpp>
#include <asmlex/lexer/Token.hpp>

#include <utility>
#include <vector>

namespace asmlex::lexer
{

  /// Column widths used by `format_token_table`.
  struct TokenTableStyle
  {
    usize kind_width = 12;
    usize text_width = 24;
    usize value_width = 20;
    bool show_positions = true;
    bool show_eof = false;
  };

  /// One aligned "kind | text | value | line:col" row per token.
  [[nodiscard]] String format_token_table(const TokenStream &stream,
                                          const TokenTableStyle &style = {});

  /// Single-line summary, e.g. "identifier 'mov'", used in tooltips.
  [[nodiscard]] String format_token_summary(const Token &token);

  /// Reconstruct source text from the stream.  Comments and newlines are
  /// emitted verbatim; every other token is followed by a single space.
  [[nodiscard]] String reconstruct_source(const TokenStream &stream);

  /// Extract the comments of a stream together with their line numbers.
  [[nodiscard]] std::vector<std::pair<u32, String>> collect_comments(const TokenStream &stream);

  /// Count tokens by kind; index the result with static_cast<usize>(kind).
  [[nodiscard]] std::vector<usize> count_by_kind(const TokenStream &stream);

  /// Locate the first token matching `kind`, or `npos` when absent.
  [[nodiscard]] usize find_token(const TokenStream &stream, TokenKind kind,
                                 usize start = 0) noexcept;

  inline constexpr usize npos_token = static_cast<usize>(-1);

} // namespace asmlex::lexer
