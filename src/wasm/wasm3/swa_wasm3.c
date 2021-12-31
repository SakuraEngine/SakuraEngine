#include "./utils.h"
#include "../common/common_utils.h"
#include "wasm/backend/wasm3/swa_wasm3.h"
#include "wasm3/wasm3.h"
#include "wasm3/m3_api_libc.h"

#define d_m3HasUVWASI
#define LINK_WASI
#include "wasm3/m3_api_wasi.h"

#define d_m3HasTracer
#include "wasm3/m3_api_tracer.h"

#include "wasm3/m3_env.h"

#define MAX_MODULES 16

const SWAProcTable tbl_wasm3 = {
    .create_instance = &swa_create_instance_wasm3,
    .free_instance = &swa_free_instance_wasm3,

    .create_runtime = &swa_create_runtime_wasm3,
    .free_runtime = &swa_free_runtime_wasm3,

    .create_module = &swa_create_module_wasm3,
    .link_host_function = &swa_module_link_host_function_wasm3,
    .free_module = &swa_free_module_wasm3,

    .exec = &swa_exec_wasm3
};

const SWAProcTable* SWA_WASM3ProcTable()
{
    return &tbl_wasm3;
}

// Instance APIs
SWAInstanceId swa_create_instance_wasm3(const struct SWAInstanceDescriptor* desc)
{
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)swa_calloc(1, sizeof(SWAInstance_WASM3));
    IW->env = m3_NewEnvironment();
    return &IW->super;
}

void swa_free_instance_wasm3(SWAInstanceId instance)
{
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)instance;
    if (IW->env) m3_FreeEnvironment(IW->env);
    swa_free(IW);
}

// Runtime APIs
SWARuntimeId swa_create_runtime_wasm3(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc)
{
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)instance;
    SWARuntime_WASM3* RW = (SWARuntime_WASM3*)swa_calloc(1, sizeof(SWARuntime_WASM3));
    RW->runtime = m3_NewRuntime(IW->env, desc->stack_size, NULL);
    RW->functions = WASM3RuntimeFunctionTableCreate();
    return &RW->super;
}

void swa_free_runtime_wasm3(SWARuntimeId runtime)
{
    SWARuntime_WASM3* RW = (SWARuntime_WASM3*)runtime;
    if (RW->runtime) m3_FreeRuntime(RW->runtime);
    if (RW->functions) WASM3RuntimeFunctionTableFree(RW->functions);
    swa_free(RW);
}

// Module APIs
SWAModuleId swa_create_module_wasm3(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc)
{
    SWARuntime_WASM3* RW = (SWARuntime_WASM3*)runtime;
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)runtime->instance;
    SWAModule_WASM3* MW = (SWAModule_WASM3*)swa_calloc(1, sizeof(SWAModule_WASM3));
    M3Result result = m3Err_none;
    result = m3_ParseModule(IW->env, &MW->module, desc->wasm, (uint32_t)desc->wasm_size);
    IM3Module module = MW->module;
    if (result) goto on_error;

    result = m3_LoadModule(RW->runtime, module);
    if (result) goto on_error;
    // Set Name
    m3_SetModuleName(module, desc->name);
    // Link spec test
    result = m3_LinkSpecTest(module);
    if (result) goto on_error;
    // Link libc
    result = m3_LinkLibC(module);
    if (result) goto on_error;
    // Link WASI
    result = m3_LinkWASI(module);
    if (result) goto on_error;
    // Link tracer
    result = m3_LinkTracer(module);
    if (result) goto on_error;
    // TODO: Link runtime functions
    return &MW->super;
on_error:
    swa_error("swa error(swa_create_module_wasm3): %s", result);
    m3_FreeModule(module);
    return SWA_NULLPTR;
}

void swa_module_link_host_function_wasm3(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc)
{
    SWAModule_WASM3* MW = (SWAModule_WASM3*)module;
    M3Result result = m3Err_none;
    result = m3_LinkRawFunctionEx(MW->module, desc->module_name,
        desc->function_name, desc->signatures.m3,
        desc->backend_wrappers.m3, desc->proc);
    if (result) goto on_error;
    return;
on_error:
    swa_error("swa error(swa_module_link_host_function_wasm3): %s, function name %s", result, desc->function_name);
}

void swa_free_module_wasm3(SWAModuleId module)
{
    SWAModule_WASM3* MW = (SWAModule_WASM3*)module;
    // m3_FreeModule(MW->module);
    m3_SetModuleName(MW->module, NULL);
    swa_free(MW);
}

// Function APIs
SWAExecResult swa_exec_wasm3(SWAModuleId module, const char8_t* const name, SWAExecDescriptor* desc)
{
    SWARuntime_WASM3* RW = (SWARuntime_WASM3*)module->runtime;
    IM3Function function = WASM3RuntimeFunctionTableTryFind(RW->functions, name);
    SWAExecResult res = SWA_NULLPTR;
    DECLARE_ZERO_VLA(const void*, ptrs, desc->param_count + 1)
    DECLARE_ZERO_VLA(const void*, out_ptrs, desc->ret_count + 1)
    if (!function)
    {
        res = m3_FindFunction(&function, RW->runtime, name);
        if (res) goto error;
        WASM3RuntimeFunctionTableAdd(RW->functions, name, function);
    }
    for (uint32_t i = 0; i < desc->param_count; i++)
        ptrs[i] = desc->params + i;
    for (uint32_t i = 0; i < desc->ret_count; i++)
        out_ptrs[i] = desc->rets + i;
    res = m3_Call(function, desc->param_count, ptrs);
    if (res) goto error;
    res = m3_GetResults(function, desc->ret_count, out_ptrs);
    if (res) goto error;
    return res;
error:
    swa_fatal("[fatal]: %s", res);
    return res;
}