#include "SkrOS/shared_memory.hpp"
#if SKR_PLAT_UNIX
#include "unix/shared_memory.cpp"
#elif SKR_PLAT_WINDOWS
#include "windows/shared_memory.cpp"
#else
#error "Unsupported platform"
#endif