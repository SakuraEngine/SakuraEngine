#include "gtest/gtest.h"
#include "wasm/api.h"
#include "../wasm.bin.h"

class WASM3Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SWAInstanceDescriptor inst_desc = { ESWA_BACKEND_WASM3 };
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

TEST_F(WASM3Test, LoadAndIncrement)
{
    SWAModuleDescriptor module_desc;
    module_desc.name = "add";
    module_desc.wasm = add_wasm;
    module_desc.wasm_size = sizeof(add_wasm);
    module_desc.bytes_pinned_outside = true;
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

TEST_F(WASM3Test, HostLink)
{
    SWAModuleDescriptor module_desc;
    module_desc.name = "invoke_host";
    module_desc.wasm = invoke_host_wasm;
    module_desc.wasm_size = sizeof(invoke_host_wasm);
    module_desc.bytes_pinned_outside = true;
    SWAModuleId module = swa_create_module(runtime, &module_desc);
    EXPECT_NE(module, nullptr);
    SWAValue param;
    param.i = 12;
    param.type = SWA_VAL_I32;
    SWAValue ret;
    SWAExecDescriptor exec_desc = {
        1, &param,
        1, &ret
    };
    SWAHostFunctionDescriptor host_func = {};
    host_func.function_name = "host_function";
    host_func.module_name = "*";
    host_func.proc = (void*)&host_function;
    host_func.signature = "i(i)";
    host_func.backend_wrappers.m3 = &host_func_warpper_m3;
    swa_module_link_host_function(module, &host_func);
    auto res = swa_exec(module, "exec", &exec_desc);
    if (res) printf("[fatal]: %s", res);
    EXPECT_EQ(res, nullptr);
    EXPECT_EQ(ret.i, 12 + 1);
}