# Module format

An ASMLEX **module** is an optional add-on that extends the studio without
being compiled into `asmlex.exe`.  Modules come in two flavours:

| Flavour    | Artifact              | When to use                                        |
| ---------- | --------------------- | -------------------------------------------------- |
| Plugin     | `*.dll`               | Adds commands, panes or importers at runtime       |
| Submodule  | directory + `CMakeLists.txt` | Adds source that is compiled into the build |

This directory holds both: `example_plugin/` is a ready-to-copy DLL template,
and each subdirectory with a `CMakeLists.txt` is picked up by the top-level
build when `ASMLEX_ENABLE_MODULES` is `ON`.

---

## Plugin entry point

A plugin is a shared library that exports exactly one symbol:

```cpp
extern "C" __declspec(dllexport)
asmlex::plugins::PluginManifest *asmlex_plugin_entry();
```

The host calls this once during startup, copies the returned manifest and keeps
the descriptor alive for the rest of the session.  The manifest declares the
commands the plugin contributes; those commands appear in the command palette
and can be bound to shortcuts like any built-in command.

`PluginHost` discovers plugins by scanning the configured plugin directory for
`*.dll`, so no registration file is needed.  See `include/asmlex/plugins/PluginHost.hpp`
for the full contract.

---

## Writing a plugin

1. Copy `example_plugin/`.
2. Rename the target in its `CMakeLists.txt`.
3. Fill in the manifest (name, version, author, description).
4. Add one `PluginCommand` per action and implement the handler.
5. Build it and drop the DLL into the plugin directory.

Plugins are loaded with `LoadLibraryW`, so they must be built against the same
C++ runtime as the host.  Building them in the same CMake tree (as the example
does) is the simplest way to guarantee that.
