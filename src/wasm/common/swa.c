#include "wasm/backend/wasm3/swa_wasm3.h"
#include "wasm/api.h"

RUNTIME_API SWAInstanceId swa_create_instance(const struct SWAInstanceDescriptor* desc)
{
    swa_assert((desc->backend == ESWA_BACKEND_WASM3) && "SWA support only wasm3 currently!");
    const SWAProcTable* tbl = SWA_NULLPTR;
    ((SWAInstanceDescriptor*)desc)->stack_size = swa_min(desc->stack_size, 64 * 1024);
    switch (desc->backend)
    {
        case ESWA_BACKEND_WASM3:
            tbl = SWA_WASM3ProcTable();
            break;
        default:
            swa_assert(0 && "unsupported swa backend!");
    }
    SWAInstance* instance = (SWAInstance*)tbl->create_instance(desc);
    instance->proc_table = tbl;
    return instance;
}

void swa_free_instance(SWAInstanceId instance)
{
    swa_assert(instance && "fatal: NULL SWA instance!");
    swa_assert(instance->proc_table->free_instance && "fatal: can't find proc free_instance!");
    instance->proc_table->free_instance(instance);
}

// Module APIs
SWAModuleId swa_create_module(SWAInstanceId instance, const struct SWAModuleDescriptor* desc)
{
    swa_assert(instance && "fatal: Called with NULL instance!");
    swa_assert(desc && "fatal: Called with NULL module descriptor!");
    swa_assert(desc->name && "WASM modules must have name!");
    swa_assert(instance->proc_table->create_module && "fatal: can't find proc create_module!");

    SWAModule* module = (SWAModule*)instance->proc_table->create_module(instance, desc);
    // Copy bytecode into module
    if (!desc->bytes_pinned_outside)
    {
        module->wasm = swa_malloc(desc->wasm_size);
        memcpy(module->wasm, desc->wasm, desc->wasm_size);
    }
    return module;
}

void swa_module_link_host_function(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc)
{
    swa_assert(module && "fatal: Called with NULL module!");
    swa_assert(module->instance && "fatal: Called with NULL instance!");
    swa_assert(module->instance->proc_table->link_host_function && "fatal: can't find proc link_host_function!");

    module->instance->proc_table->link_host_function(module, desc);
}

void swa_free_module(SWAModuleId module)
{
    swa_assert(module && "fatal: Called with NULL module!");
    swa_assert(module->instance && "fatal: NULL SWA instance!");
    swa_assert(module->instance->proc_table->free_module && "fatal: can't find proc free_module!");
    module->instance->proc_table->free_module(module);
}