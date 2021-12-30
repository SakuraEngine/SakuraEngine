#include "common_utils.h"
#include "wasm/backend/wasm3/swa_wasm3.h"
#include "wasm/backend/wasmedge/swa_wasmedge.h"
#include "wasm/api.h"

static void SWANamedRuntimeDeletor(SWANamedObjectTable* table, const char* name, void* object)
{
    swa_free_runtime((SWARuntimeId)object);
}
SWAInstanceId swa_create_instance(const struct SWAInstanceDescriptor* desc)
{
    const SWAProcTable* tbl = SWA_NULLPTR;
    switch (desc->backend)
    {
        case ESWA_BACKEND_WASM3:
            tbl = SWA_WASM3ProcTable();
            break;
        case ESWA_BACKEND_WASM_EDGE:
            tbl = SWA_WAEdgeProcTable();
            break;
        default:
            swa_assert(0 && "unsupported swa backend!");
    }
    SWAInstance* instance = (SWAInstance*)tbl->create_instance(desc);
    instance->proc_table = tbl;
    instance->runtimes = SWAObjectTableCreate();
    SWAObjectTableSetDeletor(instance->runtimes, &SWANamedRuntimeDeletor);
    return instance;
}

void swa_free_instance(SWAInstanceId instance)
{
    swa_assert(instance && "fatal: NULL SWA instance!");
    SWAObjectTableFree(instance->runtimes);
    swa_assert(instance->proc_table->free_instance && "fatal: can't find proc free_instance!");
    instance->proc_table->free_instance(instance);
}

static void SWANamedModuleDeletor(SWANamedObjectTable* table, const char* name, void* object)
{
    swa_free_module((SWAModuleId)object);
}
SWARuntimeId swa_create_runtime(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc)
{
    ((SWARuntimeDescriptor*)desc)->stack_size = swa_min(desc->stack_size, 64 * 1024);

    swa_assert(instance && "fatal: NULL SWA instance!");
    swa_assert(instance->proc_table->create_runtime && "fatal: can't find proc create_runtime!");
    SWARuntime* runtime = (SWARuntime*)instance->proc_table->create_runtime(instance, desc);
    swa_assert(runtime && "fatal: NULL runtime returned by backend!");
    runtime->proc_table = instance->proc_table;
    runtime->instance = instance;
    runtime->modules = SWAObjectTableCreate();
    SWAObjectTableSetDeletor(runtime->modules, &SWANamedModuleDeletor);
    // Insert runtime into object table
    runtime->name = SWAObjectTableAdd(instance->runtimes, desc->name, runtime);
    if (desc->name == SWA_NULLPTR)
    {
        swa_warn("warn: No name in swa runtime desc, created with fallback name %s."
                 " It's preferred to set an actual name.",
            runtime->name);
    }
    return runtime;
}

void swa_free_runtime(SWARuntimeId runtime)
{
    swa_assert(runtime && "fatal: Called with NULL runtime!");
    SWAObjectTableFree(runtime->modules);

    swa_assert(runtime->instance && "fatal: Called with NULL instance!");
    SWAObjectTableRemove(runtime->instance->runtimes, runtime->name, false);

    swa_assert(runtime->proc_table->free_runtime && "fatal: can't find proc free_runtime!");
    runtime->proc_table->free_runtime(runtime);
}

// Module APIs
SWAModuleId swa_create_module(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc)
{
    swa_assert(runtime && "fatal: Called with NULL runtime!");
    swa_assert(desc && "fatal: Called with NULL module descriptor!");
    swa_assert(desc->name && "WASM modules must have name!");
    swa_assert(runtime->proc_table->create_module && "fatal: can't find proc create_module!");

    SWAModule* module = (SWAModule*)runtime->proc_table->create_module(runtime, desc);
    if (module)
    {
        module->runtime = runtime;
        module->wasm_size = desc->wasm_size;
        // Copy bytecode into module
        module->bytes_pinned_outside = desc->bytes_pinned_outside;
        if (!desc->bytes_pinned_outside)
        {
            module->wasm = swa_malloc(module->wasm_size);
            memcpy(module->wasm, desc->wasm, module->wasm_size);
        }
        // Insert module into object table
        module->name = SWAObjectTableAdd(runtime->modules, desc->name, module);
    }
    return module;
}

void swa_module_link_host_function(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc)
{
    swa_assert(module && "fatal: Called with NULL module!");
    swa_assert(module->runtime && "fatal: Called with NULL runtime!");
    swa_assert(module->runtime->proc_table->link_host_function && "fatal: can't find proc link_host_function!");

    module->runtime->proc_table->link_host_function(module, desc);
}

void swa_free_module(SWAModuleId module)
{
    swa_assert(module && "fatal: Called with NULL module!");
    swa_assert(module->runtime && "fatal: NULL SWA runtime!");
    SWAObjectTableRemove(module->runtime->modules, module->name, false);

    swa_assert(module->runtime->proc_table->free_module && "fatal: can't find proc free_module!");
    module->runtime->proc_table->free_module(module);
    if (!module->bytes_pinned_outside)
    {
        swa_free(module->wasm);
    }
}

// Function APIs
SWAExecResult swa_exec(SWAModuleId module, const char8_t* const name, SWAExecDescriptor* desc)
{
    swa_assert(name && "fatal: Called with NULL name!");
    swa_assert(desc && "fatal: Called with NULL SWAExecDescriptor!");
    swa_assert(module && "fatal: Called with NULL module!");
    swa_assert(module->runtime && "fatal: Called with NULL runtime!");
    return module->runtime->proc_table->exec(module, name, desc);
}

// UtilX
SWARuntimeId swa_instance_try_find_runtime(SWAInstanceId instance, const char* name)
{
    return (SWARuntimeId)SWAObjectTableTryFind(instance->runtimes, name);
}

SWAModuleId swa_runtime_try_find_module(SWARuntimeId runtime, const char* name)
{
    return (SWAModuleId)SWAObjectTableTryFind(runtime->modules, name);
}
