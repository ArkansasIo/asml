// ============================================================================
//  ASMLEX - include/asmlex/plugins/PluginHost.hpp
//  Minimal out-of-tree extension point.  A plugin is a DLL that exports a
//  single `asmlex_plugin_entry` symbol and returns a manifest describing the
//  commands it adds to the IDE.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <functional>
#include <vector>

namespace asmlex::plugins
{

  /// One command contributed by a plugin, surfaced in the command palette.
  struct PluginCommand
  {
    Id id;
    String title;
    String shortcut;
  };

  /// Metadata describing a loaded plugin, filled in by the DLL.
  struct PluginManifest
  {
    String name;
    String version;
    String author;
    String description;
    std::vector<PluginCommand> commands;
  };

  /// Signature every plugin DLL must export:
  ///   extern "C" __declspec(dllexport) asmlex::plugins::PluginManifest*
  ///   asmlex_plugin_entry();
  using PluginEntry = PluginManifest *(*)();

  /// A plugin that has been located but not necessarily loaded.
  struct PluginDescriptor
  {
    String path;
    String file_name;
    bool loaded = false;
    PluginManifest manifest;
  };

  /// Discovers and loads plugin DLLs from a directory.
  class PluginHost
  {
  public:
    /// Scan `directory` for `*.dll` files and record them as descriptors.
    void scan(StringView directory);

    /// Load one previously scanned plugin by index.  Returns false and fills
    /// `error` when the DLL cannot be opened or the entry point is missing.
    bool load(usize index, String *error = nullptr);

    /// Open every scanned plugin that is not loaded yet.
    usize load_all();

    /// Forward declaration so callers can hook command execution.
    void set_command_handler(std::function<void(const PluginCommand &)> handler);

    /// Invoke the handler for the command with this id; false when no plugin
    /// advertises it.
    bool invoke(StringView command_id) const;

    [[nodiscard]] const std::vector<PluginDescriptor> &descriptors() const noexcept
    {
      return plugins_;
    }

    /// Unload every loaded plugin DLL.
    void unload_all();

  private:
    std::vector<PluginDescriptor> plugins_;
    std::vector<void *> handles_;
    std::function<void(const PluginCommand &)> handler_;
  };

} // namespace asmlex::plugins
