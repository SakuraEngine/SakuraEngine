#pragma once
#include "wasm_configure.h"

typedef struct SWAInstance SWAInstance;
typedef struct SWAInstanceDescriptor SWAInstanceDescriptor;
typedef const struct SWAInstance* SWAInstanceId;

typedef struct SWAHostFunctionDescriptor SWAHostFunctionDescriptor;
typedef struct SWAModule SWAModule;
typedef struct SWAModuleDescriptor SWAModuleDescriptor;
typedef const struct SWAModule* SWAModuleId;

typedef enum ESWABackend
{
    ESWA_BACKEND_UNDEFINED,
#ifdef USE_M3
    ESWA_BACKEND_WASM3,
#endif
    ESWA_BACKEND_EMSCRIPTON,
    ESWA_BACKEND_MAX_ENUM_BIT = 0x7FFFFFFF
} ESWABackend;

// Instance APIs
RUNTIME_API SWAInstanceId swa_create_instance(const struct SWAInstanceDescriptor* desc);
typedef SWAInstanceId (*SWAProcCreateInstance)(const struct SWAInstanceDescriptor* desc);
RUNTIME_API void swa_free_instance(SWAInstanceId instance);
typedef void (*SWAProcFreeInstance)(SWAInstanceId instance);

// Module APIs
RUNTIME_API SWAModuleId swa_create_module(SWAInstanceId instance, const struct SWAModuleDescriptor* desc);
typedef SWAModuleId (*SWAProcCreateModule)(SWAInstanceId instance, const struct SWAModuleDescriptor* desc);
RUNTIME_API void swa_module_link_host_function(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
typedef void (*SWAProcModuleLinkHostFunction)(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
RUNTIME_API void swa_free_module(SWAModuleId module);
typedef void (*SWAProcFreeModule)(SWAModuleId module);

typedef struct SWAProcTable {
    const SWAProcCreateInstance create_instance;
    const SWAProcFreeInstance free_instance;

    const SWAProcCreateModule create_module;
    const SWAProcModuleLinkHostFunction link_host_function;
    const SWAProcFreeModule free_module;
} SWAProcTable;

typedef struct SWAInstance {
    const SWAProcTable* proc_table;
} SWAInstance;

typedef struct SWAInstanceDescriptor {
    ESWABackend backend;
    uint32_t stack_size;
} SWAInstanceDescriptor;

typedef struct SWAModuleDescriptor {
    const char8_t* name;
    const uint8_t* wasm;
    size_t wasm_size;
    bool bytes_pinned_outside;
} SWAModuleDescriptor;

typedef struct SWAModule {
    SWAInstanceId instance;
    uint8_t* wasm;
    size_t wasm_size;
} SWAModule;

typedef struct SWAHostFunctionDescriptor {
    const char* const module_name;
    const char* const function_name;
    const char* const signature;
    void* proc;
} SWAHostFunctionDescriptor;