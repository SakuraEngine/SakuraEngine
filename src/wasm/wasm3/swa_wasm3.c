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

    .create_module = &swa_create_module_wasm3,
    .link_host_function = &swa_module_link_host_function_wasm3,
    .free_module = &swa_free_module_wasm3
};

const SWAProcTable* SWA_WASM3ProcTable()
{
    return &tbl_wasm3;
}

SWAInstanceId swa_create_instance_wasm3(const struct SWAInstanceDescriptor* desc)
{
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)swa_calloc(1, sizeof(SWAInstance_WASM3));
    IW->env = m3_NewEnvironment();
    IW->runtime = m3_NewRuntime(IW->env, desc->stack_size, NULL);
    return &IW->super;
}

void swa_free_instance_wasm3(SWAInstanceId instance)
{
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)instance;
    if (IW->runtime) m3_FreeRuntime(IW->runtime);
    if (IW->env) m3_FreeEnvironment(IW->env);
    swa_free(IW);
}

// Module APIs
SWAModuleId swa_create_module_wasm3(SWAInstanceId instance, const struct SWAModuleDescriptor* desc)
{
    SWAInstance_WASM3* IW = (SWAInstance_WASM3*)instance;
    SWAModule_WASM3* MW = (SWAModule_WASM3*)swa_calloc(1, sizeof(SWAModule_WASM3));
    M3Result result = m3Err_none;
    result = m3_ParseModule(IW->env, &MW->module, desc->wasm, (uint32_t)desc->wasm_size);
    IM3Module module = MW->module;
    if (result) goto on_error;

    result = m3_LoadModule(IW->runtime, module);
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
    result = m3_LinkRawFunction(MW->module, desc->module_name, desc->function_name, desc->signature, desc->proc);
    if (result) goto on_error;
on_error:
    swa_error("swa error(swa_create_module_wasm3): %s", result);
}

void swa_free_module_wasm3(SWAModuleId module)
{
    SWAModule_WASM3* MW = (SWAModule_WASM3*)module;
    // SWAInstance_WASM3* IW = (SWAInstance_WASM3*)module->instance;
    m3_FreeModule(MW->module);
    swa_free(MW);
}