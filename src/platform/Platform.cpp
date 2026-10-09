// ============================================================================
//  ASMLEX - src/platform/Platform.cpp
//  Win32 process / clipboard / message box helpers.  On other hosts these
//  degrade to the portable fallbacks so the rest of the IDE still builds.
// ============================================================================
#include <asmlex/platform/Platform.hpp>

#include <asmlex/core/FileSystem.hpp>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace asmlex::platform
{

#ifdef _WIN32
  namespace
  {
    [[nodiscard]] std::wstring widen(StringView text)
    {
      if (text.empty())
        return {};
      const int needed = ::MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                               static_cast<int>(text.size()), nullptr, 0);
      if (needed <= 0)
        return {};
      std::wstring out(static_cast<usize>(needed), L'\0');
      ::MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(),
                            needed);
      return out;
    }

    [[nodiscard]] String narrow(const std::wstring &text)
    {
      if (text.empty())
        return {};
      const int needed = ::WideCharToMultiByte(CP_UTF8, 0, text.data(),
                                               static_cast<int>(text.size()), nullptr, 0, nullptr,
                                               nullptr);
      if (needed <= 0)
        return {};
      String out(static_cast<usize>(needed), '\0');
      ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(),
                            needed, nullptr, nullptr);
      return out;
    }
  } // namespace
#endif

  ProcessResult run_process(StringView command_line, StringView working_directory)
  {
    ProcessResult result;
#ifdef _WIN32
    SECURITY_ATTRIBUTES attributes{};
    attributes.nLength = sizeof(attributes);
    attributes.bInheritHandle = TRUE;

    HANDLE read_end = nullptr;
    HANDLE write_end = nullptr;
    if (!::CreatePipe(&read_end, &write_end, &attributes, 0))
    {
      result.error = "CreatePipe failed";
      return result;
    }
    ::SetHandleInformation(read_end, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdOutput = write_end;
    startup.hStdError = write_end;
    startup.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);

    PROCESS_INFORMATION process{};
    std::wstring command = widen(command_line);
    std::wstring cwd = widen(working_directory);

    const BOOL ok = ::CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE,
                                     CREATE_NO_WINDOW, nullptr,
                                     cwd.empty() ? nullptr : cwd.c_str(), &startup, &process);
    ::CloseHandle(write_end);

    if (!ok)
    {
      ::CloseHandle(read_end);
      result.error = "CreateProcess failed";
      return result;
    }

    result.started = true;
    char buffer[4096];
    DWORD available = 0;
    for (;;)
    {
      if (!::PeekNamedPipe(read_end, nullptr, 0, nullptr, &available, nullptr))
        break;
      if (available == 0)
      {
        const DWORD wait = ::WaitForSingleObject(process.hProcess, 10);
        if (wait == WAIT_OBJECT_0 && available == 0)
        {
          DWORD leftover = 0;
          if (!::PeekNamedPipe(read_end, nullptr, 0, nullptr, &leftover, nullptr) || leftover == 0)
            break;
        }
        continue;
      }
      DWORD read = 0;
      const DWORD want = available < sizeof(buffer) ? available : static_cast<DWORD>(sizeof(buffer));
      if (!::ReadFile(read_end, buffer, want, &read, nullptr) || read == 0)
        break;
      result.stdout_text.append(buffer, read);
    }

    ::WaitForSingleObject(process.hProcess, INFINITE);
    DWORD exit_code = 0;
    ::GetExitCodeProcess(process.hProcess, &exit_code);
    result.exit_code = static_cast<int>(exit_code);

    ::CloseHandle(read_end);
    ::CloseHandle(process.hThread);
    ::CloseHandle(process.hProcess);
    return result;
#else
    const String command = String(command_line) + " 2>&1";
    FILE *pipe = ::popen(command.c_str(), "r");
    if (pipe == nullptr)
    {
      result.error = "popen failed";
      return result;
    }
    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr)
      result.stdout_text += buffer;
    result.exit_code = ::pclose(pipe);
    result.started = true;
    return result;
#endif
  }

  bool spawn_process(StringView command_line, StringView working_directory, String *error)
  {
#ifdef _WIN32
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    std::wstring command = widen(command_line);
    std::wstring cwd = widen(working_directory);

    if (!::CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, 0, nullptr,
                          cwd.empty() ? nullptr : cwd.c_str(), &startup, &process))
    {
      if (error)
        *error = "CreateProcess failed";
      return false;
    }
    ::CloseHandle(process.hThread);
    ::CloseHandle(process.hProcess);
    return true;
#else
    (void)command_line;
    (void)working_directory;
    if (error)
      *error = "spawn_process is only supported on Windows";
    return false;
#endif
  }

  bool set_clipboard_text(StringView text)
  {
#ifdef _WIN32
    if (!::OpenClipboard(nullptr))
      return false;
    ::EmptyClipboard();

    const std::wstring wide = widen(text);
    const usize bytes = (wide.size() + 1) * sizeof(wchar_t);
    HGLOBAL memory = ::GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (memory == nullptr)
    {
      ::CloseClipboard();
      return false;
    }
    void *target = ::GlobalLock(memory);
    std::memcpy(target, wide.c_str(), bytes);
    ::GlobalUnlock(memory);

    ::SetClipboardData(CF_UNICODETEXT, memory);
    ::CloseClipboard();
    return true;
#else
    (void)text;
    return false;
#endif
  }

  String clipboard_text()
  {
#ifdef _WIN32
    if (!::IsClipboardFormatAvailable(CF_UNICODETEXT) || !::OpenClipboard(nullptr))
      return {};
    HANDLE data = ::GetClipboardData(CF_UNICODETEXT);
    if (data == nullptr)
    {
      ::CloseClipboard();
      return {};
    }
    const auto *wide = static_cast<const wchar_t *>(::GlobalLock(data));
    String out = wide == nullptr ? String() : narrow(wide);
    ::GlobalUnlock(data);
    ::CloseClipboard();
    return out;
#else
    return {};
#endif
  }

  bool message_box(StringView title, StringView text, MessageBoxKind kind)
  {
#ifdef _WIN32
    UINT type = MB_OK;
    switch (kind)
    {
    case MessageBoxKind::Info:
      type |= MB_ICONINFORMATION;
      break;
    case MessageBoxKind::Warning:
      type |= MB_ICONWARNING;
      break;
    case MessageBoxKind::Error:
      type |= MB_ICONERROR;
      break;
    }
    const std::wstring wide_title = widen(title);
    const std::wstring wide_text = widen(text);
    return ::MessageBoxW(nullptr, wide_text.c_str(), wide_title.c_str(), type) == IDOK;
#else
    (void)title;
    (void)text;
    (void)kind;
    return false;
#endif
  }

  u64 monotonic_ms()
  {
    using namespace std::chrono;
    return static_cast<u64>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
  }

  String local_timestamp()
  {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    ::localtime_s(&local, &now);
#else
    ::localtime_r(&now, &local);
#endif
    char buffer[32]{};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
    return String(buffer);
  }

} // namespace asmlex::platform
