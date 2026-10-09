// ============================================================================
//  ASMLEX - src/core/Log.cpp
//  Wall clock helper backing Logger::now_ms().
// ============================================================================
#include <asmlex/core/Log.hpp>

#include <chrono>

namespace asmlex::core
{

  u64 Logger::now_ms()
  {
    using namespace std::chrono;
    return static_cast<u64>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
  }

} // namespace asmlex::core
