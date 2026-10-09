// ============================================================================
//  ASMLEX - modules/example_plugin/example_plugin.cpp
//  Template plugin.  Copy this directory, rename the target, and replace the
//  command list with your own contributions.
// ============================================================================
#include <asmlex/plugins/PluginHost.hpp>

namespace
{
  /// The manifest handed back to the host.  It must outlive the call, so it
  /// lives in a function-local static.
  asmlex::plugins::PluginManifest make_manifest()
  {
    using asmlex::plugins::PluginCommand;
    using asmlex::plugins::PluginManifest;

    PluginManifest manifest;
    manifest.name = "Example Plugin";
    manifest.version = "1.0.0";
    manifest.author = "ASMLEX";
    manifest.description = "Demonstrates the ASMLEX plugin contract.";
    manifest.commands = {
        PluginCommand{"example.hello", "Example: Say Hello", "Ctrl+Alt+H"},
        PluginCommand{"example.count_bytes", "Example: Count Selected Bytes", ""},
    };
    return manifest;
  }
} // namespace

extern "C" __declspec(dllexport) asmlex::plugins::PluginManifest *asmlex_plugin_entry()
{
  static asmlex::plugins::PluginManifest manifest = make_manifest();
  return &manifest;
}
