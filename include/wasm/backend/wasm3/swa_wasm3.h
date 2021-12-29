#pragma once
#include "wasm/api.h"
#include "wasm3/wasm3.h"

RUNTIME_API const SWAProcTable* SWA_WASM3ProcTable();

typedef struct WASM3RuntimeFunctionTable WASM3RuntimeFunctionTable;

// Instance APIs
RUNTIME_API SWAInstanceId swa_create_instance_wasm3(const struct SWAInstanceDescriptor* desc);
RUNTIME_API void swa_free_instance_wasm3(SWAInstanceId instance);

// Runtime APIs
RUNTIME_API SWARuntimeId swa_create_runtime_wasm3(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc);
RUNTIME_API void swa_free_runtime_wasm3(SWARuntimeId runtime);

// Module APIs
RUNTIME_API SWAModuleId swa_create_module_wasm3(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc);
RUNTIME_API void swa_module_link_host_function_wasm3(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
RUNTIME_API void swa_free_module_wasm3(SWAModuleId module);

// Function APIs
RUNTIME_API SWAExecResult swa_exec_wasm3(SWARuntimeId runtime, const char8_t* const name, SWAExecDescriptor* desc);

typedef struct SWAInstance_WASM3 {
    SWAInstance super;
    IM3Environment env;
} SWAInstance_WASM3;

typedef struct SWARuntime_WASM3 {
    SWARuntime super;
    IM3Runtime runtime;
    struct WASM3RuntimeFunctionTable* functions;
} SWARuntime_WASM3;

typedef struct SWAModule_WASM3 {
    SWAModule super;
    IM3Module module;
} SWAModule_WASM3;