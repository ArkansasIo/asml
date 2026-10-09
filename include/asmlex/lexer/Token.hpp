// ============================================================================
//  ASMLEX - include/asmlex/lexer/Token.hpp
//  Tokens produced by the assembly / hex lexer.
// ============================================================================
#pragma once

#include <asmlex/core/Diagnostics.hpp>
#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::lexer
{

  enum class TokenKind
  {
    EndOfFile,
    Newline,     ///< statement separator
    Identifier,  ///< label, symbol, register or mnemonic
    Integer,     ///< 123, 0x7B, 0b1010, 0o17
    Float,       ///< 1.5, 2.0e3
    String,      ///< "text" or 'text'
    HexBytes,    ///< 48 8B C1  (byte run, only in data / hex mode)
    Comment,     ///< ; comment
    Directive,   ///< .section, .org, ...
    Punctuation, ///< , : [ ] ( ) { }
    Operator,    ///< + - * / % & | ^ ~ << >> == != ...
    Unknown
  };

  [[nodiscard]] inline StringView to_string(TokenKind kind) noexcept
  {
    switch (kind)
    {
    case TokenKind::EndOfFile:
      return "eof";
    case TokenKind::Newline:
      return "newline";
    case TokenKind::Identifier:
      return "identifier";
    case TokenKind::Integer:
      return "integer";
    case TokenKind::Float:
      return "float";
    case TokenKind::String:
      return "string";
    case TokenKind::HexBytes:
      return "hexbytes";
    case TokenKind::Comment:
      return "comment";
    case TokenKind::Directive:
      return "directive";
    case TokenKind::Punctuation:
      return "punctuation";
    case TokenKind::Operator:
      return "operator";
    case TokenKind::Unknown:
      return "unknown";
    }
    return "?";
  }

  struct Token
  {
    TokenKind kind = TokenKind::Unknown;
    String text;        ///< raw lexeme exactly as written
    String value;       ///< normalised payload (string contents, directive name)
    SourceSpan where{}; ///< source location

    [[nodiscard]] bool is(TokenKind k) const noexcept { return kind == k; }
    [[nodiscard]] bool is_identifier(StringView name) const noexcept
    {
      return kind == TokenKind::Identifier && text == name;
    }
    [[nodiscard]] bool is_ci(StringView name) const noexcept
    {
      return kind == TokenKind::Identifier && upper(text) == upper(name);
    }
    [[nodiscard]] bool is_op(StringView op) const noexcept
    {
      return (kind == TokenKind::Operator || kind == TokenKind::Punctuation) && text == op;
    }
  };

  /// A lexed token stream.  Line and column information is preserved so the UI
  /// can map a caret back into the editor.
  class TokenStream
  {
  public:
    TokenStream() = default;
    explicit TokenStream(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    void push(Token t) { tokens_.push_back(std::move(t)); }

    [[nodiscard]] const std::vector<Token> &tokens() const noexcept { return tokens_; }
    [[nodiscard]] usize size() const noexcept { return tokens_.size(); }
    [[nodiscard]] bool empty() const noexcept { return tokens_.empty(); }

    [[nodiscard]] const Token &at(usize i) const noexcept
    {
      static const Token eof{};
      return i < tokens_.size() ? tokens_[i] : eof;
    }

    void clear() noexcept { tokens_.clear(); }

    /// All tokens whose kind is not `Comment`, in order.
    [[nodiscard]] TokenStream without_comments() const
    {
      TokenStream out;
      out.tokens_.reserve(tokens_.size());
      for (const Token &t : tokens_)
      {
        if (t.kind != TokenKind::Comment)
          out.tokens_.push_back(t);
      }
      return out;
    }

  private:
    std::vector<Token> tokens_{};
  };

} // namespace asmlex::lexer
