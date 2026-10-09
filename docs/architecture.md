# Architecture

ASMLEX is a layered single-binary application.  `asmlex_core` is a static
library holding every module; `asmlex.exe` is a thin Win32 entry point that
links it.

```
                         ┌──────────────┐
                         │   asmlex.exe │   src/main.cpp
                         └──────┬───────┘
                                │ links
                         ┌──────▼───────┐
                         │  asmlex_core │   src/**.cpp
                         └──────┬───────┘
        ┌───────────────────────┼───────────────────────┐
        │                       │                       │
   ┌────▼────┐            ┌─────▼─────┐          ┌──────▼──────┐
   │   ui    │            │  asm/lexer │          │ tools/etc.  │
   └────┬────┘            └─────┬─────┘          └──────┬──────┘
        └───────────────────────┼───────────────────────┘
                         ┌──────▼───────┐
                         │     core     │   types, diagnostics, log, fs
                         └──────────────┘
```

## Dependency direction

Lower layers never include higher ones.  Concretely:

- `core` depends on nothing in the project.
- `lexer`, `asm`, `types`, `nodes`, `tools`, `settings` depend on `core` only.
- `platform` depends on `core`; it is the single place that talks to Win32
  beyond `FileSystem`'s byte I/O.
- `plugins` depends on `core` and is the only module that loads foreign code.
- `ui` sits on top and is the only module allowed to touch every other one.

A violation of this ordering is a bug, not a style preference: it is what keeps
the assembler testable without a window.

## Namespaces

Each module owns a namespace named after it (`asmlex::lexer`, `asmlex::nodes`,
...).  `core/Types.hpp` re-exports the scalar aliases (`u8`, `usize`, `String`,
...) into the parent `asmlex` namespace so sibling modules can use them bare.

## Error handling

- **Expected failures** (a file is missing, a plugin has no entry point) are
  reported by return value, with an optional out-parameter carrying the reason.
- **Precondition violations** inside a module are assertions.
- Nothing throws across a module boundary except where a `bool` cannot express
  the outcome; the lexer's `DiagnosticSink` is the preferred channel.

## Threading

The message loop owns the UI.  Work that can block — assembling, launching a
child process — runs off-thread and reports back through the `Logger` sinks and
the command registry, never by touching widgets directly.
