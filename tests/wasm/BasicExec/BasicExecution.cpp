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
    SWAModuleDescriptor add_desc;
    add_desc.name = "add";
    add_desc.wasm = add_wasm;
    add_desc.wasm_size = sizeof(add_wasm);
    add_desc.bytes_pinned_outside = true;
    SWAModuleId module = swa_create_module(runtime, &add_desc);
    EXPECT_NE(module, nullptr);
    SWAValue params[2];
    params[0].i = 12;
    params[1].i = 33;
    SWAValue ret;
    SWAExecDescriptor exec_desc = {
        2, params,
        1, &ret
    };
    auto res = swa_exec(runtime, "add", &exec_desc);
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
    SWAModuleDescriptor add_desc;
    add_desc.name = "invoke_host";
    add_desc.wasm = invoke_host;
    add_desc.wasm_size = sizeof(invoke_host);
    add_desc.bytes_pinned_outside = true;
    SWAModuleId module = swa_create_module(runtime, &add_desc);
    EXPECT_NE(module, nullptr);
    SWAValue param;
    param.i = 12;
    SWAValue ret;
    SWAExecDescriptor exec_desc = {
        1, &param,
        1, &ret
    };
    SWAHostFunctionDescriptor host_func = {};
    host_func.function_name = "host_function";
    host_func.module_name = "env";
    host_func.proc = (void*)&host_function;
    host_func.signature = "i(i)";
    host_func.backend_wrappers.m3 = &host_func_warpper_m3;
    swa_module_link_host_function(module, &host_func);
    auto res = swa_exec(runtime, "exec", &exec_desc);
    if (res) printf("[fatal]: %s", res);
    EXPECT_EQ(res, nullptr);
    EXPECT_EQ(ret.i, 12 + 1);
}

TEST_F(WASM3Test, HostLinkX)
{
    SWAModuleDescriptor add_desc;
    add_desc.name = "invoke_host";
    add_desc.wasm = invoke_host;
    add_desc.wasm_size = sizeof(invoke_host);
    add_desc.bytes_pinned_outside = true;
    SWAModuleId module = swa_create_module(runtime, &add_desc);
    EXPECT_NE(module, nullptr);
    swa::utilx::link(module, "env", "host_function", host_function);
    auto ret = swa::utilx::executor(runtime, "exec").exec<int>(12);
    EXPECT_EQ(ret, 12 + 1);
}