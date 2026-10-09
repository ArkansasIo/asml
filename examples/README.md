# Examples

Sample inputs for the studio and for the test suite.  Everything here is small
enough to read in one screen.

| Path                  | Demonstrates                                        |
| --------------------- | --------------------------------------------------- |
| `hello.asmlex`        | A minimal project file (JSON)                       |
| `hello.asm`           | NASM source: exit with a constant status code       |
| `hex-payload.txt`     | The `hex` dialect — a raw byte listing              |
| `struct-point.json`   | A type table fragment with a struct and an alias    |

## Try it

```powershell
# Assemble the NASM sample, then view the encoding as hex.
asmlex examples\hello.asm --dialect nasm --emit hex

# Disassemble a raw byte run straight from the command line.
asmlex --dialect hex "48 31 C0 48 FF C0 C3"
```

The `.asmlex` file is what the GUI opens; the `.asm` and `.txt` files are plain
sources you can point the editor at directly.
