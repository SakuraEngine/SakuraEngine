#pragma once
#include "wasm_configure.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SWAInstance SWAInstance;
typedef struct SWAInstanceDescriptor SWAInstanceDescriptor;
typedef const struct SWAInstance* SWAInstanceId;

typedef struct SWARuntime SWARuntime;
typedef struct SWARuntimeDescriptor SWARuntimeDescriptor;
typedef const struct SWARuntime* SWARuntimeId;

typedef struct SWAHostFunctionDescriptor SWAHostFunctionDescriptor;
typedef struct SWAModule SWAModule;
typedef struct SWAModuleDescriptor SWAModuleDescriptor;
typedef const struct SWAModule* SWAModuleId;

typedef struct SWAFunction SWAFunction;
typedef const struct SWAFunction* SWAFunctionId;
typedef const char* SWAExecResult;

typedef union SWAValue
{
    swa_f32 f;
    swa_f64 F;
    swa_i32 i;
    swa_i64 I;
    swa_ptr ptr;
    swa_cptr cptr;
} SWAValue;

typedef struct SWAExecDescriptor {
    const uint32_t param_count;
    const SWAValue* params;
    const uint32_t ret_count;
    SWAValue* rets;
} SWAExecDescriptor;

typedef struct SWANamedObjectTable SWANamedObjectTable;

#ifdef USE_M3
    #include "wasm3/wasm3.h"
#endif
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

// Runtime APIs
RUNTIME_API SWARuntimeId swa_create_runtime(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc);
typedef SWARuntimeId (*SWAProcCreateRuntime)(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc);
RUNTIME_API void swa_free_runtime(SWARuntimeId runtime);
typedef void (*SWAProcFreeRuntime)(SWARuntimeId runtime);

// Module APIs
RUNTIME_API SWAModuleId swa_create_module(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc);
typedef SWAModuleId (*SWAProcCreateModule)(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc);
RUNTIME_API void swa_module_link_host_function(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
typedef void (*SWAProcModuleLinkHostFunction)(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
RUNTIME_API void swa_free_module(SWAModuleId module);
typedef void (*SWAProcFreeModule)(SWAModuleId module);

// Function APIs
RUNTIME_API SWAExecResult swa_exec(SWARuntimeId runtime, const char8_t* const name, SWAExecDescriptor* desc);
typedef SWAExecResult (*SWAProcExec)(SWARuntimeId runtime, const char8_t* const name, SWAExecDescriptor* desc);

// Util X
RUNTIME_API SWARuntimeId swa_instance_try_find_runtime(SWAInstanceId instance, const char* name);
RUNTIME_API SWAModuleId swa_runtime_try_find_module(SWARuntimeId runtime, const char* name);

typedef struct SWAProcTable {
    // Instance APIs
    const SWAProcCreateInstance create_instance;
    const SWAProcFreeInstance free_instance;

    // Runtime APIs
    const SWAProcCreateRuntime create_runtime;
    const SWAProcFreeRuntime free_runtime;

    // Module APIs
    const SWAProcCreateModule create_module;
    const SWAProcModuleLinkHostFunction link_host_function;
    const SWAProcFreeModule free_module;

    // Function APIs
    const SWAProcExec exec;
} SWAProcTable;

typedef struct SWAInstance {
    const SWAProcTable* proc_table;
    struct SWANamedObjectTable* runtimes;
} SWAInstance;

typedef struct SWAInstanceDescriptor {
    ESWABackend backend;
} SWAInstanceDescriptor;

typedef struct SWARuntime {
    SWAInstanceId instance;
    const char8_t* name;
    const SWAProcTable* proc_table;
    struct SWANamedObjectTable* modules;
} SWARuntime;

typedef struct SWARuntimeDescriptor {
    const char8_t* name;
    uint32_t stack_size;
} SWARuntimeDescriptor;

typedef struct SWAModuleDescriptor {
    const char8_t* name;
    const uint8_t* wasm;
    size_t wasm_size;
    bool bytes_pinned_outside;
} SWAModuleDescriptor;

typedef struct SWAModule {
    SWARuntimeId runtime;
    const char8_t* name;
    uint8_t* wasm;
    size_t wasm_size;
    bool bytes_pinned_outside;
} SWAModule;

typedef struct SWAHostFunctionDescriptor {
    const char* module_name;
    const char* function_name;
    const char* signature;
    void* proc;
    union
    {
        M3RawCall m3;
    } backend_wrappers;
} SWAHostFunctionDescriptor;

#ifdef __cplusplus
} // end extern "C"

// SWAX
namespace SWA
{

}
#endif