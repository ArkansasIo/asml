// ============================================================================
//  ASMLEX - src/core/FileSystem.cpp
//  Win32 backed file / directory helpers.  Only the calls the IDE actually
//  needs are implemented: whole-file read, whole-file write, stat and listing.
// ============================================================================
#include <asmlex/core/FileSystem.hpp>

#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace asmlex::core
{

  namespace
  {
    /// Convert a UTF-8 std::string into a wide buffer for the Win32 APIs.
#ifdef _WIN32
    [[nodiscard]] std::wstring widen(StringView text)
    {
      if (text.empty())
        return {};
      const int needed = ::MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                               static_cast<int>(text.size()), nullptr, 0);
      if (needed <= 0)
        return {};
      std::wstring out(static_cast<usize>(needed), L'\0');
      ::MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                            out.data(), needed);
      return out;
    }
#endif

    [[nodiscard]] String win32_error_text(unsigned long code)
    {
#ifdef _WIN32
      LPWSTR buffer = nullptr;
      const DWORD n = ::FormatMessageW(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
              FORMAT_MESSAGE_IGNORE_INSERTS,
          nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
      if (n == 0 || buffer == nullptr)
        return "unknown error";

      int needed = ::WideCharToMultiByte(CP_UTF8, 0, buffer, static_cast<int>(n), nullptr, 0,
                                         nullptr, nullptr);
      String out(static_cast<usize>(needed), '\0');
      ::WideCharToMultiByte(CP_UTF8, 0, buffer, static_cast<int>(n), out.data(), needed, nullptr,
                            nullptr);
      ::LocalFree(buffer);
      while (!out.empty() && (out.back() == '\n' || out.back() == '\r'))
        out.pop_back();
      return out;
#else
      (void)code;
      return "unknown error";
#endif
    }
  } // namespace

  FileReadResult read_file(StringView path)
  {
    FileReadResult result;

#ifdef _WIN32
    const std::wstring wide = widen(path);
    if (wide.empty())
    {
      result.error = "empty path";
      return result;
    }

    HANDLE handle = ::CreateFileW(wide.c_str(), GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
    {
      result.error = win32_error_text(::GetLastError());
      return result;
    }

    LARGE_INTEGER size{};
    if (!::GetFileSizeEx(handle, &size))
    {
      result.error = win32_error_text(::GetLastError());
      ::CloseHandle(handle);
      return result;
    }

    result.contents.resize(static_cast<usize>(size.QuadPart));
    usize total = 0;
    while (total < result.contents.size())
    {
      DWORD chunk = 0;
      const DWORD want = static_cast<DWORD>(result.contents.size() - total);
      if (!::ReadFile(handle, result.contents.data() + total, want, &chunk, nullptr))
      {
        result.error = win32_error_text(::GetLastError());
        ::CloseHandle(handle);
        return result;
      }
      if (chunk == 0)
        break;
      total += chunk;
    }
    result.contents.resize(total);
    ::CloseHandle(handle);
    result.ok = true;
    return result;
#else
    FILE *fp = std::fopen(String(path).c_str(), "rb");
    if (!fp)
    {
      result.error = "cannot open file";
      return result;
    }
    std::fseek(fp, 0, SEEK_END);
    const long length = std::ftell(fp);
    std::fseek(fp, 0, SEEK_SET);
    if (length > 0)
    {
      result.contents.resize(static_cast<usize>(length));
      result.contents.resize(std::fread(result.contents.data(), 1, result.contents.size(), fp));
    }
    std::fclose(fp);
    result.ok = true;
    return result;
#endif
  }

  bool write_file(StringView path, StringView contents, String *error)
  {
#ifdef _WIN32
    const std::wstring wide = widen(path);
    if (wide.empty())
    {
      if (error)
        *error = "empty path";
      return false;
    }

    HANDLE handle = ::CreateFileW(wide.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
    {
      if (error)
        *error = win32_error_text(::GetLastError());
      return false;
    }

    usize written = 0;
    bool ok = true;
    while (written < contents.size())
    {
      DWORD chunk = 0;
      const DWORD want = static_cast<DWORD>(contents.size() - written);
      if (!::WriteFile(handle, contents.data() + written, want, &chunk, nullptr))
      {
        if (error)
          *error = win32_error_text(::GetLastError());
        ok = false;
        break;
      }
      written += chunk;
    }
    ::CloseHandle(handle);
    return ok;
#else
    FILE *fp = std::fopen(String(path).c_str(), "wb");
    if (!fp)
    {
      if (error)
        *error = "cannot create file";
      return false;
    }
    const usize n = std::fwrite(contents.data(), 1, contents.size(), fp);
    std::fclose(fp);
    if (n != contents.size())
    {
      if (error)
        *error = "short write";
      return false;
    }
    return true;
#endif
  }

  bool file_exists(StringView path)
  {
#ifdef _WIN32
    const std::wstring wide = widen(path);
    if (wide.empty())
      return false;
    const DWORD attrs = ::GetFileAttributesW(wide.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
    FILE *fp = std::fopen(String(path).c_str(), "rb");
    if (!fp)
      return false;
    std::fclose(fp);
    return true;
#endif
  }

  bool directory_exists(StringView path)
  {
#ifdef _WIN32
    const std::wstring wide = widen(path);
    if (wide.empty())
      return false;
    const DWORD attrs = ::GetFileAttributesW(wide.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
    (void)path;
    return false;
#endif
  }

  u64 file_size(StringView path)
  {
#ifdef _WIN32
    const std::wstring wide = widen(path);
    if (wide.empty())
      return 0;
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!::GetFileAttributesExW(wide.c_str(), GetFileExInfoStandard, &data))
      return 0;
    return (static_cast<u64>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
#else
    (void)path;
    return 0;
#endif
  }

  u64 file_modified_ms(StringView path)
  {
#ifdef _WIN32
    const std::wstring wide = widen(path);
    if (wide.empty())
      return 0;
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!::GetFileAttributesExW(wide.c_str(), GetFileExInfoStandard, &data))
      return 0;
    ULARGE_INTEGER stamp{};
    stamp.LowPart = data.ftLastWriteTime.dwLowDateTime;
    stamp.HighPart = data.ftLastWriteTime.dwHighDateTime;
    // FILETIME is 100-ns ticks since 1601; shift to unix milliseconds.
    constexpr u64 kEpochDelta = 11644473600000ull;
    return (stamp.QuadPart / 10000ull) - kEpochDelta;
#else
    (void)path;
    return 0;
#endif
  }

  std::vector<String> list_directory(StringView directory, bool want_directories)
  {
    std::vector<String> out;
#ifdef _WIN32
    String pattern(directory);
    if (!pattern.empty() && pattern.back() != '\\' && pattern.back() != '/')
      pattern.push_back('\\');
    pattern += '*';

    WIN32_FIND_DATAW data{};
    HANDLE handle = ::FindFirstFileW(widen(pattern).c_str(), &data);
    if (handle == INVALID_HANDLE_VALUE)
      return out;

    do
    {
      const std::wstring name(data.cFileName);
      if (name == L"." || name == L"..")
        continue;
      const bool is_dir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
      if (is_dir != want_directories)
        continue;
      const int needed = ::WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, nullptr, 0, nullptr,
                                               nullptr);
      if (needed <= 1)
        continue;
      String entry(static_cast<usize>(needed - 1), '\0');
      ::WideCharToMultiByte(CP_UTF8, 0, data.cFileName, -1, entry.data(), needed, nullptr, nullptr);
      out.push_back(std::move(entry));
    } while (::FindNextFileW(handle, &data));

    ::FindClose(handle);
#else
    (void)directory;
    (void)want_directories;
#endif
    return out;
  }

  String file_name(StringView path)
  {
    const usize slash = path.find_last_of("/\\");
    return slash == StringView::npos ? String(path) : String(path.substr(slash + 1));
  }

  String file_extension(StringView path)
  {
    const String name = file_name(path);
    const usize dot = name.find_last_of('.');
    if (dot == String::npos)
      return {};
    return lower(name.substr(dot));
  }

  String strip_extension(StringView path)
  {
    const String name = file_name(path);
    const usize dot = name.find_last_of('.');
    if (dot == String::npos)
      return String(path);
    const usize slash = path.find_last_of("/\\");
    const usize base = slash == StringView::npos ? 0 : slash + 1;
    return String(path.substr(0, base + dot));
  }

  String parent_directory(StringView path)
  {
    const usize slash = path.find_last_of("/\\");
    if (slash == StringView::npos)
      return ".";
    if (slash == 0)
      return String(path.substr(0, 1));
    return String(path.substr(0, slash));
  }

  String path_join(StringView a, StringView b)
  {
    if (a.empty())
      return String(b);
    if (b.empty())
      return String(a);
    String out(a);
    const char back = out.back();
    const char front = b.front();
    const bool has_sep = (back == '/' || back == '\\');
    const bool wants_sep = (front == '/' || front == '\\');
    if (!has_sep && !wants_sep)
      out.push_back('/');
    out.append(b);
    return out;
  }

  String normalize_slashes(StringView path)
  {
    String out(path);
    for (char &c : out)
    {
      if (c == '\\')
        c = '/';
    }
    return out;
  }

} // namespace asmlex::core
