#include "SkrBase/containers/string/literal.hpp"
#include "SkrBase/misc/debug.h"

// platform specific imports
#if SKR_PLAT_WINDOWS
    #include <psapi.h>
    #include <processthreadsapi.h>
    #include <libloaderapi.h>
#elif SKR_PLAT_UNIX
extern char etext, edata, end;
#endif

namespace skr
{
bool in_const_segment(const void* ptr)
{
#if SKR_PLAT_WINDOWS
    static MODULEINFO module_info;
    static const bool b = GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(NULL), &module_info, sizeof(MODULEINFO));
    SKR_ASSERT(b && "Failed to get module information");
    return uintptr_t(ptr) > uintptr_t(module_info.EntryPoint) &&
           uintptr_t(ptr) < (uintptr_t(module_info.EntryPoint) + module_info.SizeOfImage);
#elif SKR_PLAT_UNIX
    return (ptr > &etext) && (ptr < &edata);
#else
    static const char*     test_str              = "__A UNIQUE CONST SEGMENT STRING__";
    static const uintptr_t PROBED_CONST_SEG_ADDR = uintptr_t(test_str);

    return (std::labs(static_cast<long>(uintptr_t(ptr) - PROBED_CONST_SEG_ADDR)) < 500'0000L);
#endif
}
} // namespace skr