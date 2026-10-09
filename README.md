# ASMLEX

**Assembler / Disassembler Studio** — a Windows desktop IDE for writing,
assembling, disassembling, running and debugging low level assembly, built on a
single C++23 code base.

---

## Building

ASMLEX builds with CMake 3.21+ and a C++23 compiler.  On Windows the helper
script locates MSVC through `vswhere`, imports the developer environment and
drives CMake + Ninja — no *Developer Command Prompt* required.

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Debug -Run
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Clean
```

Anything CMake accepts works too:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target asmlex
```

The executable lands in `build/bin/asmlex.exe`.

---

## Layout

| Path                  | Contents                                                  |
| --------------------- | --------------------------------------------------------- |
| `include/asmlex/`     | Public headers, one directory per module                  |
| `src/`                | Module implementations (`asmlex_core` static library)     |
| `src/main.cpp`        | Win32 entry point, links against `asmlex_core`            |
| `modules/`            | Optional out-of-tree modules and example plugin projects  |
| `assets/`             | Icons, fonts, syntax themes and other runtime resources   |
| `docs/`               | Design notes and user documentation                       |
| `examples/`           | Sample `.asmlex` projects and assembly sources            |
| `scripts/`            | Build and packaging helpers                               |
| `third_party/jsoncpp` | Vendored JSON library (MIT / public domain)               |

### Modules

| Module     | Responsibility                                                                |
| ---------- | ----------------------------------------------------------------------------- |
| `core`     | Value types, diagnostics, logging, file system, product identity              |
| `lexer`    | Tokenizer, expression evaluation, source manager, token printing              |
| `asm`      | Instruction set tables and instruction encoding metadata                      |
| `nodes`    | Generic node graph shared by the listing and the type editor                  |
| `types`    | Project type table (primitives, pointers, arrays, aggregates)                 |
| `settings` | Persisted user preferences (JSON in the user profile)                          |
| `plugins`  | Out-of-tree DLL extensions and their command contributions                    |
| `platform` | OS services: process launch, clipboard, message boxes, clock                  |
| `tools`    | Hex dump / byte parsing helpers used by the viewer and the CLI                |
| `ui`       | Command registry backing the menu bar, toolbar and command palette            |

Each module follows the same shape: a header in `include/asmlex/<module>/` and
an implementation in `src/<module>/`.  The CMake target globs `src/*.cpp`, so a
new module only needs a header and a `.cpp` to join the build.

---

## Third-party

`third_party/jsoncpp` vendors [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
1.9.5 (MIT / public domain).  The sources are kept flat next to the CMake target
that compiles them; the public headers are also reachable through `json/` so
upstream-style `#include <json/value.h>` keeps working.

The generated `version.h` that upstream's CMake would emit is not present, so
the version numbers and the `JSONCPP_USING_SECURE_MEMORY` switch are defined
inline in `config.h`.

---

## License

See the individual third-party licenses under `third_party/`.  Project sources
are distributed under the terms in `docs/LICENSE` once added.
