// ============================================================================
//  ASMLEX - src/lexer/Expr.cpp
//  Constant expression evaluator (precedence climbing / recursive descent).
// ============================================================================
#include <asmlex/lexer/Expr.hpp>
#include <asmlex/lexer/Lexer.hpp>

namespace asmlex::lexer
{

  bool parse_integer_literal(StringView text, i64 &out)
  {
    String s(text);
    // strip an optional trailing size suffix (e.g. 12h, 0FFh, 10b)
    if (!s.empty())
    {
      const char last = s.back();
      if (last == 'h' || last == 'H')
      {
        String body = s.substr(0, s.size() - 1);
        if (!body.empty() && body[0] >= '0' && body[0] <= '9' &&
            !(body.size() > 1 && (body[1] == 'x' || body[1] == 'X')))
        {
          i64 v = 0;
          for (char c : body)
          {
            const int d = (c >= '0' && c <= '9')   ? (c - '0')
                          : (c >= 'a' && c <= 'f') ? (c - 'a' + 10)
                          : (c >= 'A' && c <= 'F') ? (c - 'A' + 10)
                                                   : -1;
            if (d < 0)
              return false;
            v = (v << 4) | d;
          }
          out = v;
          return true;
        }
      }
    }

    for (char &c : s)
    {
      if (c == '_')
        c = ' ';
    }
    {
      String cleaned;
      for (char c : s)
      {
        if (c != ' ')
          cleaned.push_back(c);
      }
      s = std::move(cleaned);
    }

    int base = 10;
    usize start = 0;
    if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
    {
      base = 16;
      start = 2;
    }
    else if (s.size() > 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B'))
    {
      base = 2;
      start = 2;
    }
    else if (s.size() > 2 && s[0] == '0' && (s[1] == 'o' || s[1] == 'O'))
    {
      base = 8;
      start = 2;
    }
    if (start >= s.size())
      return false;

    u64 acc = 0;
    bool any = false;
    for (usize i = start; i < s.size(); ++i)
    {
      const char c = s[i];
      int d = -1;
      if (c >= '0' && c <= '9')
        d = c - '0';
      else if (c >= 'a' && c <= 'f')
        d = c - 'a' + 10;
      else if (c >= 'A' && c <= 'F')
        d = c - 'A' + 10;
      else
        return false;
      if (d >= base)
        return false;
      acc = acc * static_cast<u64>(base) + static_cast<u64>(d);
      any = true;
    }
    if (!any)
      return false;
    out = static_cast<i64>(acc);
    return true;
  }

  ExprValue ExpressionParser::error(StringView text, usize pos)
  {
    const Token &t = peek(pos);
    sink_.error("E1001", String(text), t.where);
    ExprValue v;
    v.unresolved = true;
    return v;
  }

  bool ExpressionParser::accept_op(usize &pos, StringView op)
  {
    const Token &t = peek(pos);
    if ((t.kind == TokenKind::Operator || t.kind == TokenKind::Punctuation) && t.text == op)
    {
      ++pos;
      return true;
    }
    return false;
  }

  bool ExpressionParser::accept_kw(usize &pos, StringView kw)
  {
    const Token &t = peek(pos);
    if (t.kind == TokenKind::Identifier && upper(t.text) == upper(kw))
    {
      ++pos;
      return true;
    }
    return false;
  }

  ExprValue ExpressionParser::fold(const ExprValue &a, const ExprValue &b, StringView op,
                                   u32 line, u32 column)
  {
    if (a.unresolved)
      return a;
    if (b.unresolved)
      return b;

    ExprValue r;
    const i64 x = a.value;
    const i64 y = b.value;

    if (op == "+")
      r.value = x + y;
    else if (op == "-")
      r.value = x - y;
    else if (op == "*")
      r.value = x * y;
    else if (op == "/")
    {
      if (y == 0)
      {
        sink_.error("E1002", "division by zero in constant expression",
                    SourceSpan{"", SourcePos{line, column, 0}, SourcePos{}});
        r.unresolved = true;
        return r;
      }
      r.value = x / y;
    }
    else if (op == "%")
    {
      if (y == 0)
      {
        sink_.error("E1002", "modulo by zero in constant expression",
                    SourceSpan{"", SourcePos{line, column, 0}, SourcePos{}});
        r.unresolved = true;
        return r;
      }
      r.value = x % y;
    }
    else if (op == "&")
      r.value = x & y;
    else if (op == "|")
      r.value = x | y;
    else if (op == "^")
      r.value = x ^ y;
    else if (op == "<<")
      r.value = (y >= 0 && y < 64) ? static_cast<i64>(static_cast<u64>(x) << y) : 0;
    else if (op == ">>")
      r.value = (y >= 0 && y < 64) ? static_cast<i64>(static_cast<u64>(x) >> y) : 0;
    else if (op == "==")
      r.value = (x == y) ? 1 : 0;
    else if (op == "!=")
      r.value = (x != y) ? 1 : 0;
    else if (op == "<")
      r.value = (x < y) ? 1 : 0;
    else if (op == "<=")
      r.value = (x <= y) ? 1 : 0;
    else if (op == ">")
      r.value = (x > y) ? 1 : 0;
    else if (op == ">=")
      r.value = (x >= y) ? 1 : 0;
    else if (op == "&&")
      r.value = (x && y) ? 1 : 0;
    else if (op == "||")
      r.value = (x || y) ? 1 : 0;
    else
    {
      sink_.error("E1003", String("unknown operator '") + String(op) + "' in expression");
      r.unresolved = true;
      return r;
    }
    r.is_float = a.is_float || b.is_float;
    return r;
  }

  ExprValue ExpressionParser::parse(usize &pos) { return parse_or(pos); }

  ExprValue ExpressionParser::parse_or(usize &pos)
  {
    ExprValue left = parse_and(pos);
    while (accept_op(pos, "||"))
    {
      ExprValue right = parse_and(pos);
      left = fold(left, right, "||", 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_and(usize &pos)
  {
    ExprValue left = parse_bitor(pos);
    while (accept_op(pos, "&&"))
    {
      ExprValue right = parse_bitor(pos);
      left = fold(left, right, "&&", 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_bitor(usize &pos)
  {
    ExprValue left = parse_bitxor(pos);
    while (peek(pos).is_op("|") && !peek(pos).is_op("||"))
    {
      ++pos;
      ExprValue right = parse_bitxor(pos);
      left = fold(left, right, "|", 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_bitxor(usize &pos)
  {
    ExprValue left = parse_bitand(pos);
    while (peek(pos).is_op("^"))
    {
      ++pos;
      ExprValue right = parse_bitand(pos);
      left = fold(left, right, "^", 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_bitand(usize &pos)
  {
    ExprValue left = parse_shift(pos);
    while (peek(pos).is_op("&") && !peek(pos).is_op("&&"))
    {
      ++pos;
      ExprValue right = parse_shift(pos);
      left = fold(left, right, "&", 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_shift(usize &pos)
  {
    ExprValue left = parse_additive(pos);
    while (peek(pos).is_op("<<") || peek(pos).is_op(">>"))
    {
      const String op = peek(pos).text;
      ++pos;
      ExprValue right = parse_additive(pos);
      left = fold(left, right, op, 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_additive(usize &pos)
  {
    ExprValue left = parse_term(pos);
    while (peek(pos).is_op("+") || peek(pos).is_op("-"))
    {
      const String op = peek(pos).text;
      ++pos;
      ExprValue right = parse_term(pos);
      left = fold(left, right, op, 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_term(usize &pos)
  {
    ExprValue left = parse_unary(pos);
    while (peek(pos).is_op("*") || peek(pos).is_op("/") || peek(pos).is_op("%"))
    {
      const String op = peek(pos).text;
      ++pos;
      ExprValue right = parse_unary(pos);
      left = fold(left, right, op, 0, 0);
    }
    return left;
  }

  ExprValue ExpressionParser::parse_unary(usize &pos)
  {
    if (accept_op(pos, "-"))
    {
      ExprValue v = parse_unary(pos);
      if (!v.unresolved)
        v.value = -v.value;
      return v;
    }
    if (accept_op(pos, "+"))
      return parse_unary(pos);
    if (accept_op(pos, "~"))
    {
      ExprValue v = parse_unary(pos);
      if (!v.unresolved)
        v.value = ~v.value;
      return v;
    }
    if (accept_op(pos, "!"))
    {
      ExprValue v = parse_unary(pos);
      if (!v.unresolved)
        v.value = v.value ? 0 : 1;
      return v;
    }
    return parse_primary(pos);
  }

  ExprValue ExpressionParser::parse_primary(usize &pos)
  {
    const Token &t = peek(pos);

    if (t.is_op("("))
    {
      ++pos;
      ExprValue v = parse_or(pos);
      if (!accept_op(pos, ")"))
        return error("expected ')' in expression", pos);
      return v;
    }

    if (t.kind == TokenKind::Integer)
    {
      ++pos;
      ExprValue v;
      if (!parse_integer_literal(t.text, v.value))
      {
        sink_.error("E1004", String("malformed integer literal '") + t.text + "'", t.where);
        v.unresolved = true;
      }
      return v;
    }

    if (t.kind == TokenKind::Float)
    {
      ++pos;
      ExprValue v;
      v.is_float = true;
      try
      {
        v.value = static_cast<i64>(std::stod(t.text));
      }
      catch (...)
      {
        sink_.error("E1005", String("malformed float literal '") + t.text + "'", t.where);
        v.unresolved = true;
      }
      return v;
    }

    if (t.kind == TokenKind::Identifier)
    {
      ++pos;
      // built-in pseudo-values
      const String u = upper(t.text);
      if (u == "$" || u == "$$" || u == "HERE")
      {
        ExprValue v;
        v.value = 0; // patched by the assembler once the location is known
        v.unresolved = true;
        v.symbol = t.text;
        return v;
      }

      if (resolver_)
      {
        i64 resolved = 0;
        if (resolver_(t.text, resolved))
        {
          ExprValue v;
          v.value = resolved;
          return v;
        }
      }

      ExprValue v;
      v.unresolved = true;
      v.symbol = t.text;
      return v;
    }

    return error("expected a value in expression", pos);
  }

  ExprValue evaluate_expression(StringView text, DiagnosticSink &sink,
                                const SymbolResolver &resolver, Dialect dialect)
  {
    LexOptions options;
    options.dialect = dialect;
    options.file = "<expr>";
    Lexer lexer(std::move(options), sink);
    TokenStream stream = lexer.run(text);
    ExpressionParser parser(stream, sink);
    if (resolver)
      parser.set_resolver(resolver);
    return parser.parse_all();
  }

} // namespace asmlex::lexer
