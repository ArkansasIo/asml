# Assets

Runtime resources shipped next to `asmlex.exe`.  Nothing here is compiled; the
build copies this directory to the output tree.

| Directory  | Contents                                                        |
| ---------- | --------------------------------------------------------------- |
| `icons/`   | Application, toolbar and file-type icons (`.ico`, `.png`)       |
| `themes/`  | Syntax highlighting themes as JSON                               |
| `fonts/`   | Bundled monospaced fonts (only when licensing permits)           |
| `samples/` | Tiny inputs used by the welcome screen and the self-tests        |

## Theme format

A theme is a JSON object mapping token categories to `#RRGGBB` colours:

```json
{
  "name": "ASMLEX Dark",
  "base": "dark",
  "colors": {
    "background": "#1E1E1E",
    "foreground": "#D4D4D4",
    "keyword": "#569CD6",
    "register": "#4EC9B0",
    "number": "#B5CEA8",
    "string": "#CE9178",
    "comment": "#6A9955",
    "directive": "#C586C0",
    "label": "#DCDCAA",
    "error": "#F44747"
  }
}
```

Any category omitted falls back to `foreground`.  `base` selects which built-in
palette the editor derives its chrome (gutter, selection, caret) from.
