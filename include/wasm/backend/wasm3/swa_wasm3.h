#pragma once
#include "wasm/api.h"
#include "wasm3/wasm3.h"

RUNTIME_API const SWAProcTable* SWA_WASM3ProcTable();

// Instance APIs
RUNTIME_API SWAInstanceId swa_create_instance_wasm3(const struct SWAInstanceDescriptor* desc);
RUNTIME_API void swa_free_instance_wasm3(SWAInstanceId instance);

// Module APIs
RUNTIME_API SWAModuleId swa_create_module_wasm3(SWAInstanceId instance, const struct SWAModuleDescriptor* desc);
RUNTIME_API void swa_module_link_host_function_wasm3(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
RUNTIME_API void swa_free_module_wasm3(SWAModuleId module);

typedef struct SWAInstance_WASM3 {
    SWAInstance super;
    IM3Environment env;
    IM3Runtime runtime;
} SWAInstance_WASM3;

typedef struct SWAModule_WASM3 {
    SWAModule super;
    IM3Module module;
} SWAModule_WASM3;