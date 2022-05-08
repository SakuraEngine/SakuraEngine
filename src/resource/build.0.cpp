#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include "windows.h"
#endif
#include "resource_factory.cpp"
#include "resource_system.cpp"
#include "config_resource.cpp"
#include "local_resource_registry.cpp"
#include "resource_handle.cpp"