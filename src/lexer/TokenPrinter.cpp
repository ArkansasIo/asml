// ============================================================================
//  ASMLEX - src/lexer/TokenPrinter.cpp
//  Text rendering for token streams (lexer pane + CLI --dump-tokens).
// ============================================================================
#include <asmlex/lexer/TokenPrinter.hpp>

#include <utility>

namespace asmlex::lexer
{

  String format_token_summary(const Token &token)
  {
    String out(to_string(token.kind));
    out += " '";
    out += token.text;
    out += '\'';
    return out;
  }

  String format_token_table(const TokenStream &stream, const TokenTableStyle &style)
  {
    String out;
    out += core::column("KIND", style.kind_width);
    out += ' ';
    out += core::column("TEXT", style.text_width);
    out += ' ';
    out += core::column("VALUE", style.value_width);
    if (style.show_positions)
    {
      out += ' ';
      out += "LINE:COL";
    }
    out += '\n';

    const usize rule = style.kind_width + style.text_width + style.value_width + 3 +
                       (style.show_positions ? 9 : 0);
    out.append(rule, '-');
    out += '\n';

    for (const Token &token : stream.tokens())
    {
      if (token.kind == TokenKind::EndOfFile && !style.show_eof)
        continue;

      out += core::column(to_string(token.kind), style.kind_width);
      out += ' ';
      out += core::column(token.text, style.text_width);
      out += ' ';
      out += core::column(token.value, style.value_width);
      if (style.show_positions)
      {
        out += ' ';
        out += std::to_string(token.where.begin.line);
        out += ':';
        out += std::to_string(token.where.begin.column);
      }
      out += '\n';
    }
    return out;
  }

  String reconstruct_source(const TokenStream &stream)
  {
    String out;
    for (const Token &token : stream.tokens())
    {
      switch (token.kind)
      {
      case TokenKind::EndOfFile:
        break;
      case TokenKind::Newline:
        out += '\n';
        break;
      case TokenKind::Comment:
        out += token.text;
        break;
      default:
        out += token.text;
        out += ' ';
        break;
      }
    }
    return out;
  }

  std::vector<std::pair<u32, String>> collect_comments(const TokenStream &stream)
  {
    std::vector<std::pair<u32, String>> out;
    for (const Token &token : stream.tokens())
    {
      if (token.kind == TokenKind::Comment)
        out.emplace_back(token.where.begin.line, token.text);
    }
    return out;
  }

  std::vector<usize> count_by_kind(const TokenStream &stream)
  {
    constexpr usize kKindCount = static_cast<usize>(TokenKind::Unknown) + 1;
    std::vector<usize> counts(kKindCount, 0);
    for (const Token &token : stream.tokens())
    {
      const usize index = static_cast<usize>(token.kind);
      if (index < counts.size())
        ++counts[index];
    }
    return counts;
  }

  usize find_token(const TokenStream &stream, TokenKind kind, usize start) noexcept
  {
    const auto &tokens = stream.tokens();
    for (usize i = start; i < tokens.size(); ++i)
    {
      if (tokens[i].kind == kind)
        return i;
    }
    return npos_token;
  }

} // namespace asmlex::lexer
