#pragma once
#include "wasm/api.h"
#include "wasmedge/wasmedge.h"

RUNTIME_API const SWAProcTable* SWA_WAEdgeProcTable();

// Instance APIs
RUNTIME_API SWAInstanceId swa_create_instance_WAEdge(const struct SWAInstanceDescriptor* desc);
RUNTIME_API void swa_free_instance_WAEdge(SWAInstanceId instance);

// Runtime APIs
RUNTIME_API SWARuntimeId swa_create_runtime_WAEdge(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc);
RUNTIME_API void swa_free_runtime_WAEdge(SWARuntimeId runtime);

// Module APIs
RUNTIME_API SWAModuleId swa_create_module_WAEdge(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc);
RUNTIME_API void swa_module_link_host_function_WAEdge(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc);
RUNTIME_API void swa_free_module_WAEdge(SWAModuleId module);

// Function APIs
RUNTIME_API SWAExecResult swa_exec_WAEdge(SWAModuleId module, const char8_t* const name, SWAExecDescriptor* desc);

typedef struct SWAInstance_WAEdge {
    SWAInstance super;
    WasmEdge_ConfigureContext* cfg;
} SWAInstance_WAEdge;

typedef struct SWARuntime_WAEdge {
    SWARuntime super;
    WasmEdge_StatisticsContext* stat_ctx;
    WasmEdge_StoreContext* store;
    WasmEdge_LoaderContext* load_ctx;
    WasmEdge_ValidatorContext* valid_ctx;
    WasmEdge_ExecutorContext* exec_ctx;
} SWARuntime_WAEdge;

typedef struct SWAModule_WAEdge {
    SWAModule super;
    WasmEdge_String mod_name;
    WasmEdge_ASTModuleContext* ast_ctx;
} SWAModule_WAEdge;