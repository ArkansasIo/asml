// ============================================================================
//  ASMLEX - src/plugins/PluginHost.cpp
//  Win32 DLL discovery + load.  On non-Windows hosts the class still builds
//  but scan/load are no-ops so the UI layer needs no #ifdefs of its own.
// ============================================================================
#include <asmlex/plugins/PluginHost.hpp>

#include <asmlex/core/FileSystem.hpp>
#include <asmlex/core/Log.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace asmlex::plugins
{

  void PluginHost::scan(StringView directory)
  {
    plugins_.clear();
    if (!core::directory_exists(directory))
    {
      ASMLEX_LOG_WARN("plugin directory missing: ", directory);
      return;
    }

    for (const String &name : core::list_directory(directory, false))
    {
      if (lower(core::file_extension(name)) != ".dll")
        continue;
      PluginDescriptor descriptor;
      descriptor.path = core::path_join(directory, name);
      descriptor.file_name = name;
      plugins_.push_back(std::move(descriptor));
    }
    ASMLEX_LOG_INFO("plugin scan found ", plugins_.size(), " candidate(s)");
  }

  bool PluginHost::load(usize index, String *error)
  {
    if (index >= plugins_.size())
    {
      if (error)
        *error = "plugin index out of range";
      return false;
    }
    PluginDescriptor &descriptor = plugins_[index];
    if (descriptor.loaded)
      return true;

#ifdef _WIN32
    HMODULE module = ::LoadLibraryW(std::wstring(descriptor.path.begin(),
                                                 descriptor.path.end())
                                        .c_str());
    if (module == nullptr)
    {
      if (error)
        *error = "LoadLibrary failed for " + descriptor.path;
      return false;
    }

    auto entry = reinterpret_cast<PluginEntry>(::GetProcAddress(module, "asmlex_plugin_entry"));
    if (entry == nullptr)
    {
      if (error)
        *error = "missing asmlex_plugin_entry in " + descriptor.file_name;
      ::FreeLibrary(module);
      return false;
    }

    if (PluginManifest *manifest = entry())
      descriptor.manifest = *manifest;

    handles_.push_back(module);
    descriptor.loaded = true;
    ASMLEX_LOG_INFO("loaded plugin '", descriptor.manifest.name, "' ", descriptor.manifest.version);
    return true;
#else
    (void)descriptor;
    if (error)
      *error = "plugin loading is only supported on Windows";
    return false;
#endif
  }

  usize PluginHost::load_all()
  {
    usize loaded = 0;
    for (usize i = 0; i < plugins_.size(); ++i)
    {
      String error;
      if (load(i, &error))
        ++loaded;
      else
        ASMLEX_LOG_WARN("plugin load failed: ", error);
    }
    return loaded;
  }

  void PluginHost::set_command_handler(std::function<void(const PluginCommand &)> handler)
  {
    handler_ = std::move(handler);
  }

  bool PluginHost::invoke(StringView command_id) const
  {
    if (!handler_)
      return false;
    for (const PluginDescriptor &descriptor : plugins_)
    {
      for (const PluginCommand &command : descriptor.manifest.commands)
      {
        if (command.id == command_id)
        {
          handler_(command);
          return true;
        }
      }
    }
    return false;
  }

  void PluginHost::unload_all()
  {
#ifdef _WIN32
    for (void *handle : handles_)
    {
      if (handle != nullptr)
        ::FreeLibrary(static_cast<HMODULE>(handle));
    }
#endif
    handles_.clear();
    for (PluginDescriptor &descriptor : plugins_)
      descriptor.loaded = false;
  }

} // namespace asmlex::plugins
