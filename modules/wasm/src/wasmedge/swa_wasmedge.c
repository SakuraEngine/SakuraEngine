#include "../common/common_utils.h"
#include "wasm/backend/wasmedge/swa_wasmedge.h"

const SWAProcTable tbl_WAEdge = {
    .create_instance = &swa_create_instance_WAEdge,
    .free_instance = &swa_free_instance_WAEdge,

    .create_runtime = &swa_create_runtime_WAEdge,
    .free_runtime = &swa_free_runtime_WAEdge,

    .create_module = &swa_create_module_WAEdge,
    .link_host_function = &swa_module_link_host_function_WAEdge,
    .free_module = &swa_free_module_WAEdge,

    .exec = &swa_exec_WAEdge
};

const SWAProcTable* SWA_WAEdgeProcTable()
{
    return &tbl_WAEdge;
}

// Instance APIs
SWAInstanceId swa_create_instance_WAEdge(const struct SWAInstanceDescriptor* desc)
{
    SWAInstance_WAEdge* IW = (SWAInstance_WAEdge*)swa_calloc(1, sizeof(SWAInstance_WAEdge));
    IW->cfg = WasmEdge_ConfigureCreate();
    // Add propsals
    WasmEdge_ConfigureAddProposal(IW->cfg, WasmEdge_Proposal_SIMD);
    WasmEdge_ConfigureAddProposal(IW->cfg, WasmEdge_Proposal_Memory64);
    WasmEdge_ConfigureAddProposal(IW->cfg, WasmEdge_Proposal_ImportExportMutGlobals);
    // Add host registration
    WasmEdge_ConfigureAddHostRegistration(IW->cfg, WasmEdge_HostRegistration_Wasi);
    return &IW->super;
}

void swa_free_instance_WAEdge(SWAInstanceId instance)
{
    SWAInstance_WAEdge* IW = (SWAInstance_WAEdge*)instance;
    WasmEdge_ConfigureDelete(IW->cfg);
    swa_free(IW);
}

// Runtime APIs
SWARuntimeId swa_create_runtime_WAEdge(SWAInstanceId instance, const struct SWARuntimeDescriptor* desc)
{
    SWAInstance_WAEdge* IW = (SWAInstance_WAEdge*)instance;
    SWARuntime_WAEdge* RW = (SWARuntime_WAEdge*)swa_calloc(1, sizeof(SWARuntime_WAEdge));
    RW->store = WasmEdge_StoreCreate();
    RW->stat_ctx = WasmEdge_StatisticsCreate();
    RW->load_ctx = WasmEdge_LoaderCreate(IW->cfg);
    RW->valid_ctx = WasmEdge_ValidatorCreate(IW->cfg);
    RW->exec_ctx = WasmEdge_ExecutorCreate(IW->cfg, RW->stat_ctx);
    return &RW->super;
}

void swa_free_runtime_WAEdge(SWARuntimeId runtime)
{
    SWARuntime_WAEdge* RW = (SWARuntime_WAEdge*)runtime;
    WasmEdge_StoreDelete(RW->store);
    WasmEdge_LoaderDelete(RW->load_ctx);
    WasmEdge_StatisticsDelete(RW->stat_ctx);
    WasmEdge_ValidatorDelete(RW->valid_ctx);
    WasmEdge_ExecutorDelete(RW->exec_ctx);
    swa_free(RW);
}

// Module APIs
SWAModuleId swa_create_module_WAEdge(SWARuntimeId runtime, const struct SWAModuleDescriptor* desc)
{
    SWARuntime_WAEdge* RW = (SWARuntime_WAEdge*)runtime;
    WasmEdge_Result result;
    SWAModule_WAEdge* MW = (SWAModule_WAEdge*)swa_calloc(1, sizeof(SWAModule_WAEdge));

    MW->mod_name = WasmEdge_StringCreateByCString(desc->name);
    result = WasmEdge_LoaderParseFromBuffer(RW->load_ctx, &MW->ast_ctx, desc->wasm, desc->wasm_size);
    if (!WasmEdge_ResultOK(result)) goto on_error;
    result = WasmEdge_ValidatorValidate(RW->valid_ctx, MW->ast_ctx);
    if (!WasmEdge_ResultOK(result)) goto on_error;
    if (desc->strong_stub)
    {
        result = WasmEdge_ExecutorRegisterModule(RW->exec_ctx, RW->store, MW->ast_ctx, MW->mod_name);
        if (!WasmEdge_ResultOK(result)) goto on_error;
        MW->super.instantiated = true;
    }
    return &MW->super;
on_error:
    swa_error("swa error(swa_create_module_WAEdge): %s", WasmEdge_ResultGetMessage(result));
    WasmEdge_StringDelete(MW->mod_name);
    swa_free(MW);
    return SWA_NULLPTR;
}

void swa_module_link_host_function_WAEdge(SWAModuleId module, const struct SWAHostFunctionDescriptor* desc)
{
    SWARuntime_WAEdge* RW = (SWARuntime_WAEdge*)module->runtime;
    WasmEdge_Result result;

    WasmEdge_String module_name = WasmEdge_StringCreateByCString(desc->module_name);
    WasmEdge_ImportObjectContext* import_module = WasmEdge_ImportObjectCreate(module_name);
    WasmEdge_StringDelete(module_name);

    WasmEdge_FunctionTypeContext* func_t_ctx = WasmEdge_FunctionTypeCreate(
        desc->signatures.wa_edge.i_types, desc->signatures.wa_edge.i_count,
        desc->signatures.wa_edge.o_types, desc->signatures.wa_edge.o_count);
    WasmEdge_FunctionInstanceContext* host_func =
        WasmEdge_FunctionInstanceCreate(func_t_ctx, desc->backend_wrappers.wa_edge, NULL, 0);
    WasmEdge_FunctionTypeDelete(func_t_ctx);

    WasmEdge_String import_name = WasmEdge_StringCreateByCString(desc->function_name);
    WasmEdge_ImportObjectAddFunction(import_module, import_name, host_func);
    WasmEdge_StringDelete(import_name);

    // Register host function
    result = WasmEdge_ExecutorRegisterImport(RW->exec_ctx, RW->store, import_module);
    if (!WasmEdge_ResultOK(result)) goto on_error;
    return;
on_error:
    WasmEdge_FunctionInstanceDelete(host_func);
    swa_error("swa error(swa_link_host_WAEdge): %s", WasmEdge_ResultGetMessage(result));
}

void swa_free_module_WAEdge(SWAModuleId module)
{
    SWAModule_WAEdge* MW = (SWAModule_WAEdge*)module;
    WasmEdge_ASTModuleDelete(MW->ast_ctx);
    WasmEdge_StringDelete(MW->mod_name);
    swa_free(MW);
}

// Function APIs
SWAExecResult swa_exec_WAEdge(SWAModuleId module, const char8_t* const name, SWAExecDescriptor* desc)
{
    SWAModule_WAEdge* MW = (SWAModule_WAEdge*)module;
    SWARuntime_WAEdge* RW = (SWARuntime_WAEdge*)module->runtime;
    WasmEdge_Result result;
    // Find in store
    {
        const WasmEdge_String mod_name = MW->mod_name;
        WasmEdge_String func_name = WasmEdge_StringCreateByCString(name);
        DECLARE_ZERO_VLA(WasmEdge_Value, params, desc->param_count + 1)
        DECLARE_ZERO_VLA(WasmEdge_Value, rets, desc->ret_count + 1)
        for (uint32_t i = 0; i < desc->param_count; i++)
        {
            switch (desc->params[i].type)
            {
                case SWA_VAL_I32:
                    params[i] = WasmEdge_ValueGenI32(desc->params[i].i);
                    break;
                case SWA_VAL_I64:
                    params[i] = WasmEdge_ValueGenI64(desc->params[i].I);
                    break;
                case SWA_VAL_F32:
                    params[i] = WasmEdge_ValueGenF32(desc->params[i].f);
                    break;
                case SWA_VAL_F64:
                    params[i] = WasmEdge_ValueGenF64(desc->params[i].F);
                    break;
                default:
                    swa_assert(0 && "not supported!");
                    break;
            }
        }
        if (module->strong_stub)
        {
            swa_assert(module->instantiated && "error: strong stub must be instantiated!");
            result = WasmEdge_ExecutorInvokeRegistered(
                RW->exec_ctx, RW->store,
                mod_name, func_name,
                params, desc->param_count, rets, desc->ret_count);
            if (!WasmEdge_ResultOK(result)) goto on_error;
        }
        else
        {
            if (!module->instantiated)
            {
                result = WasmEdge_ExecutorInstantiate(RW->exec_ctx, RW->store, MW->ast_ctx);
                if (!WasmEdge_ResultOK(result)) goto on_error;
                MW->super.instantiated = true;
            }
            result = WasmEdge_ExecutorInvoke(RW->exec_ctx, RW->store, func_name,
                params, desc->param_count, rets, desc->ret_count);
            if (!WasmEdge_ResultOK(result)) goto on_error;
        }
        for (uint32_t i = 0; i < desc->ret_count; i++)
        {
            switch (rets[i].Type)
            {
                case WasmEdge_ValType_I32: {
                    desc->rets[i].type = SWA_VAL_I32;
                    desc->rets[i].i = WasmEdge_ValueGetI32(rets[i]);
                }
                break;
                case WasmEdge_ValType_I64: {
                    desc->rets[i].type = SWA_VAL_I64;
                    desc->rets[i].I = WasmEdge_ValueGetI64(rets[i]);
                }
                break;
                case WasmEdge_ValType_F32: {
                    desc->rets[i].type = SWA_VAL_F32;
                    desc->rets[i].f = WasmEdge_ValueGetF32(rets[i]);
                }
                break;
                case WasmEdge_ValType_F64: {
                    desc->rets[i].type = SWA_VAL_F64;
                    desc->rets[i].F = WasmEdge_ValueGetF64(rets[i]);
                }
                break;
                default:
                    swa_assert(0 && "not supported!");
                    break;
            }
        }
        WasmEdge_StringDelete(func_name);
    }
    return SWA_NULLPTR;
on_error:
    swa_fatal("[fatal]: %s", WasmEdge_ResultGetMessage(result));
    return WasmEdge_ResultGetMessage(result);
}