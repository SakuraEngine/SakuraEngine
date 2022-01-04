#include "gtest/gtest.h"
#include "wasm/api.h"
#include "../wasm.bin.h"

class WASM3Test : public ::testing::TestWithParam<ESWABackend>
{
protected:
    void SetUp() override
    {
        ESWABackend backend = GetParam();
        SWAInstanceDescriptor inst_desc = { backend };
        instance = swa_create_instance(&inst_desc);
        EXPECT_NE(instance, nullptr);
        SWARuntimeDescriptor runtime_desc = { "wa_runtime", 64 * 1024 };
        runtime = swa_create_runtime(instance, &runtime_desc);
        EXPECT_NE(runtime, nullptr);
    }

    void TearDown() override
    {
        if (runtime) swa_free_runtime(runtime);
        if (instance) swa_free_instance(instance);
    }
    SWAInstanceId instance = nullptr;
    SWARuntimeId runtime = nullptr;
};

TEST_P(WASM3Test, LoadAndIncrement)
{
    SWAModuleDescriptor module_desc = {};
    module_desc.name = "add";
    module_desc.wasm = add_wasm;
    module_desc.wasm_size = sizeof(add_wasm);
    module_desc.bytes_pinned_outside = true;
    module_desc.strong_stub = false;
    SWAModuleId module = swa_create_module(runtime, &module_desc);
    EXPECT_NE(module, nullptr);
    SWAValue params[2];
    params[0].i = 12;
    params[0].type = SWA_VAL_I32;
    params[1].i = 33;
    params[1].type = SWA_VAL_I32;
    SWAValue ret;
    SWAExecDescriptor exec_desc = {
        2, params,
        1, &ret
    };
    auto res = swa_exec(module, "add", &exec_desc);
    EXPECT_EQ(res, nullptr);
    EXPECT_EQ(ret.i, 12 + 33);
}

int host_function(int val)
{
    printf("Hello Host! %d\n", val);
    return val + 1;
}

m3ApiRawFunction(host_func_warpper_m3)
{
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, val);
    int ret = host_function(val);
    m3ApiReturn(ret);
}

WasmEdge_Result host_func_warpper_edge(void*, WasmEdge_MemoryInstanceContext*,
    const WasmEdge_Value* In, WasmEdge_Value* Out)
{
    /*
     * Params: {i32}
     * Returns: {i32}
     * Developers should take care about the function type.
     */
    /* Retrieve the value 1. */
    int32_t val = WasmEdge_ValueGetI32(In[0]);
    /* Output value 1 is Val1 + Val2. */
    Out[0] = WasmEdge_ValueGenI32(host_function(val));
    /* Return the status of success. */
    return WasmEdge_Result_Success;
}

TEST_P(WASM3Test, HostLink)
{
    SWAModuleDescriptor module_desc;
    module_desc.name = "invoke_host";
    module_desc.wasm = invoke_host_wasm;
    module_desc.wasm_size = sizeof(invoke_host_wasm);
    module_desc.bytes_pinned_outside = true;
    module_desc.strong_stub = false;
    SWAModuleId module = swa_create_module(runtime, &module_desc);
    EXPECT_NE(module, nullptr);

    SWAHostFunctionDescriptor host_func = {};
    host_func.function_name = "host_function";
    host_func.module_name = "*";
    host_func.proc = (void*)&host_function;
    host_func.signatures.m3 = "i(i)";
    host_func.backend_wrappers.m3 = &host_func_warpper_m3;
    host_func.backend_wrappers.wa_edge = &host_func_warpper_edge;
    const auto type = WasmEdge_ValType_I32;
    host_func.signatures.wa_edge.i_count = 1;
    host_func.signatures.wa_edge.o_count = 1;
    host_func.signatures.wa_edge.i_types = &type;
    host_func.signatures.wa_edge.o_types = &type;
    swa_module_link_host_function(module, &host_func);

    SWAValue param;
    param.i = 12;
    param.type = SWA_VAL_I32;
    SWAValue ret;
    SWAExecDescriptor exec_desc = {
        1, &param,
        1, &ret
    };
    auto res = swa_exec(module, "exec", &exec_desc);
    if (res) printf("[fatal]: %s", res);
    EXPECT_EQ(res, nullptr);
    EXPECT_EQ(ret.i, 12 + 1);

    param.i = 15;
    res = swa_exec(module, "exec", &exec_desc);
    if (res) printf("[fatal]: %s", res);
    EXPECT_EQ(res, nullptr);
    EXPECT_EQ(ret.i, 15 + 1);
}

static const auto allPlatforms = testing::Values(
#ifdef USE_M3
    ESWA_BACKEND_WASM3
#endif
#ifdef USE_WASM_EDGE
    ,
    ESWA_BACKEND_WASM_EDGE
#endif
);

INSTANTIATE_TEST_SUITE_P(WASM3Test, WASM3Test, allPlatforms);