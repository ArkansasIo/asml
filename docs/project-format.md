# Project format

An ASMLEX project is a JSON document with the extension `.asmlex`.  The layout
is versioned by `core::kProjectFormatVersion`; a reader must refuse a file whose
`format_version` is newer than the one it knows.

```json
{
  "format_version": 1,
  "name": "hello",
  "target": {
    "architecture": "x86-64",
    "bits": 64,
    "format": "pe",
    "base_address": "0x140000000"
  },
  "dialect": "nasm",
  "sources": [
    { "path": "src/hello.asm", "role": "assembly" },
    { "path": "src/data.asm",  "role": "data" }
  ],
  "types": [
    { "name": "Point", "kind": "struct", "size": 8,
      "fields": [
        { "name": "x", "type": "i32", "offset": 0 },
        { "name": "y", "type": "i32", "offset": 4 }
      ] }
  ],
  "settings": {
    "assembler": "nasm",
    "optimize": false,
    "listing": true
  }
}
```

## Fields

| Key              | Type    | Notes                                                       |
| ---------------- | ------- | ----------------------------------------------------------- |
| `format_version` | integer | Required.  See `kProjectFormatVersion`.                     |
| `name`           | string  | Shown in the title bar and the recent list.                 |
| `target`         | object  | Architecture, bit width, object format and load address.    |
| `dialect`        | string  | `nasm`, `gas`, `masm` or `hex`.                             |
| `sources`        | array   | Each entry has a `path` and a `role`.                       |
| `types`          | array   | The project type table, mirroring `asmlex::types::TypeDef`.  |
| `settings`       | object  | Free-form; unknown keys are preserved on save.              |

Paths are relative to the project file and always stored with `/` separators so
a project moves between hosts unchanged.

## Reading and writing

```cpp
asmlex::types::TypeSystem types;
types.load_builtins();
asmlex::settings::Settings prefs =
    asmlex::settings::load(asmlex::settings::default_settings_path());
```

Session state (open tabs, caret positions, panel sizes) is *not* part of the
project.  It is written to `*.asmlex-state.json` beside the project and is
covered by `.gitignore`.
