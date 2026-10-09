// ============================================================================
//  ASMLEX - src/lexer/Lexer.cpp
//  Hand written scanner for the ASMLEX assembly dialects.
// ============================================================================
#include <asmlex/lexer/Lexer.hpp>

#include <array>
#include <string_view>

namespace asmlex::lexer
{

  namespace
  {
    [[nodiscard]] bool is_digit(char c) noexcept { return c >= '0' && c <= '9'; }
    [[nodiscard]] bool is_hex(char c) noexcept
    {
      return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }
    [[nodiscard]] bool is_alpha(char c) noexcept
    {
      return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }
    [[nodiscard]] bool is_ident_start(char c) noexcept
    {
      return is_alpha(c) || c == '_' || c == '.' || c == '?' || c == '@' || c == '$';
    }
    [[nodiscard]] bool is_ident_char(char c) noexcept
    {
      return is_ident_start(c) || is_digit(c);
    }
    [[nodiscard]] bool is_space(char c) noexcept
    {
      return c == ' ' || c == '\t' || c == '\f' || c == '\v';
    }

    inline constexpr std::array<std::string_view, 64> kRegisters = {
        "al", "ah", "bl", "bh", "cl", "ch", "dl", "dh",
        "ax", "bx", "cx", "dx", "si", "di", "bp", "sp",
        "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
        "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b",
        "rip", "eip", "ip", "flags", "eflags", "rflags", "cs", "xmm0"};

    inline constexpr std::array<std::string_view, 26> kDirectives = {
        ".section", ".segment", ".text", ".data", ".bss",
        ".org", ".align", ".db", ".dw", ".dd", ".dq",
        ".byte", ".word", ".long", ".quad", ".ascii", ".asciz",
        ".string", ".space", ".zero", ".equ", ".set", ".globl",
        ".global", ".type", ".entry"};

    /// True when the character can start a "48 8B C1" style byte run.
    [[nodiscard]] bool looks_like_byte_run(StringView source, usize i) noexcept
    {
      usize j = i;
      while (j < source.size() && is_space(source[j]))
        ++j;
      if (j + 1 >= source.size())
        return false;
      if (!is_hex(source[j]) || !is_hex(source[j + 1]))
        return false;
      const char after = (j + 2 < source.size()) ? source[j + 2] : '\0';
      return after == '\0' || is_space(after) || after == ',';
    }
  } // namespace

  bool Lexer::is_register(StringView name)
  {
    const String lowered = lower(name);
    for (StringView reg : kRegisters)
    {
      if (reg == lowered)
        return true;
    }
    return false;
  }

  bool Lexer::is_directive(StringView name)
  {
    const String lowered = lower(name);
    for (StringView dir : kDirectives)
    {
      if (dir == lowered)
        return true;
    }
    return false;
  }

  bool Lexer::match_hex_bytes(StringView source, usize i) const
  {
    if (!options_.allow_hex_bytes || options_.dialect != Dialect::Hex)
      return false;
    return looks_like_byte_run(source, i);
  }

  Dialect Lexer::detect_dialect(StringView source, StringView extension)
  {
    const String ext = lower(extension);
    if (ext == ".s" || ext == ".gas")
      return Dialect::Gas;
    if (ext == ".asm" || ext == ".masm")
      return Dialect::Masm;
    if (ext == ".hex")
      return Dialect::Hex;

    bool saw_directive = false;
    bool saw_percent = false;
    bool saw_bracket_colon = false;
    usize hex_pairs = 0;
    usize other = 0;

    for (usize i = 0; i < source.size() && i < 4096; ++i)
    {
      const char c = source[i];
      if (c == '\n')
        continue;
      if (c == '%')
        saw_percent = true;
      if (c == '.' && i + 1 < source.size() && is_alpha(source[i + 1]))
        saw_directive = true;
      if (c == '[' && i + 1 < source.size() && source[i + 1] == ']')
        saw_bracket_colon = true;
      if (is_hex(c) && (i + 2 >= source.size() || is_space(source[i + 2])))
        ++hex_pairs;
      else if (!is_space(c))
        ++other;
    }

    if (saw_percent)
      return Dialect::Gas;
    if (hex_pairs > 8 && other * 3 < hex_pairs)
      return Dialect::Hex;
    if (!saw_directive && saw_bracket_colon)
      return Dialect::Masm;
    return Dialect::Nasm;
  }

  Token Lexer::lex_number(StringView source, usize &i, SourcePos pos)
  {
    Token t;
    t.kind = TokenKind::Integer;
    t.where.begin = pos;
    const usize start = i;

    bool is_float = false;
    if (source[i] == '0' && i + 1 < source.size() &&
        (source[i + 1] == 'x' || source[i + 1] == 'X'))
    {
      i += 2;
      while (i < source.size() && (is_hex(source[i]) || source[i] == '_'))
        ++i;
    }
    else if (source[i] == '0' && i + 1 < source.size() &&
             (source[i + 1] == 'b' || source[i + 1] == 'B'))
    {
      i += 2;
      while (i < source.size() && (source[i] == '0' || source[i] == '1' || source[i] == '_'))
        ++i;
    }
    else if (source[i] == '0' && i + 1 < source.size() &&
             (source[i + 1] == 'o' || source[i + 1] == 'O'))
    {
      i += 2;
      while (i < source.size() && (source[i] >= '0' && source[i] <= '7' || source[i] == '_'))
        ++i;
    }
    else
    {
      while (i < source.size() && (is_digit(source[i]) || source[i] == '_'))
        ++i;
      if (i < source.size() && source[i] == '.' && (i + 1 >= source.size() || is_digit(source[i + 1])))
      {
        is_float = true;
        ++i;
        while (i < source.size() && (is_digit(source[i]) || source[i] == '_'))
          ++i;
      }
      if (i < source.size() && (source[i] == 'e' || source[i] == 'E'))
      {
        const usize save = i;
        ++i;
        if (i < source.size() && (source[i] == '+' || source[i] == '-'))
          ++i;
        if (i < source.size() && is_digit(source[i]))
        {
          is_float = true;
          while (i < source.size() && is_digit(source[i]))
            ++i;
        }
        else
        {
          i = save;
        }
      }
    }

    t.text = String(source.substr(start, i - start));
    t.value = t.text;
    t.kind = is_float ? TokenKind::Float : TokenKind::Integer;
    t.where.end.offset = i;
    t.where.end.line = pos.line;
    t.where.end.column = pos.column + static_cast<u32>(i - start);
    t.where.file = options_.file;
    return t;
  }

  Token Lexer::lex_identifier(StringView source, usize &i, SourcePos pos)
  {
    Token t;
    t.kind = TokenKind::Identifier;
    t.where.begin = pos;
    const usize start = i;
    while (i < source.size() && is_ident_char(source[i]))
      ++i;
    t.text = String(source.substr(start, i - start));
    t.value = t.text;
    t.where.end.offset = i;
    t.where.end.line = pos.line;
    t.where.end.column = pos.column + static_cast<u32>(i - start);
    t.where.file = options_.file;
    return t;
  }

  Token Lexer::lex_string(StringView source, usize &i, SourcePos pos)
  {
    Token t;
    t.kind = TokenKind::String;
    t.where.begin = pos;
    const char quote = source[i];
    const usize start = i;
    ++i;
    String value;
    while (i < source.size() && source[i] != quote)
    {
      if (source[i] == '\\' && i + 1 < source.size())
      {
        ++i;
        switch (source[i])
        {
        case 'n':
          value.push_back('\n');
          break;
        case 't':
          value.push_back('\t');
          break;
        case 'r':
          value.push_back('\r');
          break;
        case '0':
          value.push_back('\0');
          break;
        case '\\':
          value.push_back('\\');
          break;
        case '"':
          value.push_back('"');
          break;
        case '\'':
          value.push_back('\'');
          break;
        default:
          value.push_back(source[i]);
          break;
        }
      }
      else
      {
        value.push_back(source[i]);
      }
      ++i;
    }
    if (i < source.size())
      ++i; // consume the closing quote
    t.text = String(source.substr(start, i - start));
    t.value = std::move(value);
    t.where.end.offset = i;
    t.where.end.line = pos.line;
    t.where.end.column = pos.column + static_cast<u32>(i - start);
    t.where.file = options_.file;
    return t;
  }

  Token Lexer::lex_directive(StringView source, usize &i, SourcePos pos)
  {
    Token t;
    t.kind = TokenKind::Directive;
    t.where.begin = pos;
    const usize start = i;
    ++i; // skip '.'
    while (i < source.size() && is_ident_char(source[i]))
      ++i;
    t.text = String(source.substr(start, i - start));
    t.value = lower(t.text);
    t.where.end.offset = i;
    t.where.end.line = pos.line;
    t.where.end.column = pos.column + static_cast<u32>(i - start);
    t.where.file = options_.file;
    return t;
  }

  TokenStream Lexer::run(StringView source)
  {
    TokenStream stream;
    usize i = 0;
    u32 line = 1;
    u32 col = 1;

    auto advance = [&](usize n)
    {
      for (usize k = 0; k < n && i < source.size(); ++k)
      {
        if (source[i] == '\n')
        {
          ++line;
          col = 1;
        }
        else
        {
          ++col;
        }
        ++i;
      }
    };

    auto make_pos = [&]() -> SourcePos
    { return SourcePos{line, col, i}; };

    while (i < source.size())
    {
      const char c = source[i];

      if (c == '\n')
      {
        Token t;
        t.kind = TokenKind::Newline;
        t.text = "\n";
        t.where.begin = make_pos();
        t.where.file = options_.file;
        advance(1);
        t.where.end = make_pos();
        stream.push(std::move(t));
        continue;
      }

      if (is_space(c))
      {
        advance(1);
        continue;
      }

      // comments: ';' everywhere, '#' in gas, "//" tolerated.
      const bool gas_comment = (options_.dialect == Dialect::Gas && c == '#');
      const bool slash_comment = (c == '/' && i + 1 < source.size() && source[i + 1] == '/');
      if (c == ';' || gas_comment || slash_comment)
      {
        const SourcePos pos = make_pos();
        const usize start = i;
        while (i < source.size() && source[i] != '\n')
          ++i;
        if (options_.keep_comments)
        {
          Token t;
          t.kind = TokenKind::Comment;
          t.text = String(source.substr(start, i - start));
          t.value = t.text;
          t.where.begin = pos;
          t.where.end = make_pos();
          t.where.file = options_.file;
          stream.push(std::move(t));
        }
        continue;
      }

      // byte runs in hex mode: "48 8B C1"
      if (match_hex_bytes(source, i))
      {
        const SourcePos pos = make_pos();
        const usize start = i;
        while (i < source.size())
        {
          if (is_space(source[i]) || source[i] == ',')
          {
            ++i;
            continue;
          }
          if (is_hex(source[i]) && looks_like_byte_run(source, i))
          {
            ++i;
            ++i;
            continue;
          }
          break;
        }
        Token t;
        t.kind = TokenKind::HexBytes;
        t.text = String(source.substr(start, i - start));
        t.value = t.text;
        t.where.begin = pos;
        t.where.end = make_pos();
        t.where.file = options_.file;
        stream.push(std::move(t));
        continue;
      }

      // directives start with '.' followed by a letter
      if (c == '.' && i + 1 < source.size() && is_alpha(source[i + 1]))
      {
        Token t = lex_directive(source, i, make_pos());
        stream.push(std::move(t));
        col = t.where.end.column;
        continue;
      }

      if (is_digit(c) || (c == '.' && i + 1 < source.size() && is_digit(source[i + 1])))
      {
        Token t = lex_number(source, i, make_pos());
        stream.push(std::move(t));
        col = t.where.end.column;
        continue;
      }

      if (c == '"' || c == '\'')
      {
        Token t = lex_string(source, i, make_pos());
        stream.push(std::move(t));
        col = t.where.end.column;
        continue;
      }

      if (is_ident_start(c))
      {
        Token t = lex_identifier(source, i, make_pos());
        stream.push(std::move(t));
        col = t.where.end.column;
        continue;
      }

      // multi-character operators first
      const SourcePos pos = make_pos();
      const usize remaining = source.size() - i;
      const usize two = remaining >= 2 ? 2 : 1;
      const StringView op(source.data() + i, two);
      usize width = 1;
      TokenKind kind = TokenKind::Unknown;

      if (two == 2 &&
          (op == "<<" || op == ">>" || op == "==" || op == "!=" ||
           op == "<=" || op == ">=" || op == "&&" || op == "||"))
      {
        width = 2;
        kind = TokenKind::Operator;
      }
      else if (c == ',' || c == '[' || c == ']' || c == '(' || c == ')' ||
               c == '{' || c == '}' || c == ':')
      {
        kind = TokenKind::Punctuation;
      }
      else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '&' ||
               c == '|' || c == '^' || c == '~' || c == '<' || c == '>' || c == '=' || c == '!')
      {
        kind = TokenKind::Operator;
      }

      Token t;
      t.kind = kind;
      t.text = String(source.substr(i, width));
      t.value = t.text;
      t.where.begin = pos;
      t.where.file = options_.file;
      advance(width);
      t.where.end = make_pos();
      stream.push(std::move(t));
    }

    Token eof;
    eof.kind = TokenKind::EndOfFile;
    eof.where.begin = SourcePos{line, col, i};
    eof.where.end = eof.where.begin;
    eof.where.file = options_.file;
    stream.push(std::move(eof));
    return stream;
  }

} // namespace asmlex::lexer
