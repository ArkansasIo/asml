# Language support

## Dialects

| Dialect | `to_string` | Notes                                                     |
| ------- | ----------- | --------------------------------------------------------- |
| NASM    | `nasm`      | Reference syntax.  `;` comments, `section`/`global`.      |
| GAS     | `gas`       | AT&T operand order, `#` comments, `.`-prefixed directives. |
| MASM    | `masm`      | Intel order, `;` comments, `PROC`/`ENDP` blocks.          |
| Hex     | `hex`       | Pure `48 8B C1` byte listings — no mnemonics.             |

`Lexer::detect_dialect(source, extension)` picks the best fit when a file is
opened; an explicit choice in the project file always wins.

## Lexical rules

- **Identifiers** start with a letter, `_`, `.` or `@` and may contain digits.
- **Numbers** accept `0x`/`0X` hex, `0b` binary, `0o` octal, a trailing `h`/`b`
  marker, and `_` as a digit separator.
- **Strings** are `"..."` or `'...'` with `\n`, `\t`, `\\`, `\"` and `\xNN`
  escapes.  Unterminated strings are reported at the opening quote.
- **Comments** run to end of line; character set depends on the dialect.
- **Hex byte runs** are recognised only when the dialect is `hex` or
  `LexOptions::allow_hex_bytes` is set.

## Diagnostics

Every token carries a `SourcePos` (file, line, column).  The lexer pushes to a
`DiagnosticSink` rather than throwing, so a file with several errors reports all
of them in one pass.  Severity and message text come from
`include/asmlex/core/Diagnostics.hpp`.

## Registers and directives

`Lexer::is_register` and `Lexer::is_directive` answer against the built-in
x86-64 profile described in `include/asmlex/asm/InstructionSet.hpp`.  These are
used for syntax highlighting and operand classification, not for validation —
the assembler performs the authoritative check.
