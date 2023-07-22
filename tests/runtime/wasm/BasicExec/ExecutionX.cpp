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

int host_function_2(uint8_t val)
{
    printf("Hello Host! %d\n", val);
    return val + 2;
}

TEST_F(WASM3Test, HostLinkX)
{
    SWAModuleDescriptor module_desc;
    module_desc.name = "invoke_host";
    module_desc.wasm = invoke_host_wasm;
    module_desc.wasm_size = sizeof(invoke_host_wasm);
    module_desc.bytes_pinned_outside = true;
    SWAModuleId module = swa_create_module(runtime, &module_desc);
    EXPECT_NE(module, nullptr);
    swa::utilx::link(module, "*", "host_function", host_function_2);
    auto ret = swa::utilx::executor(module, "exec").exec<int>(12);
    EXPECT_EQ(ret, 12 + 2);
}

int host_function_u8(uint8_t val)
{
    printf("Hello Host! %d\n", val);
    return val + 1;
}

m3ApiRawFunction(m3_libc_memset)
{
    m3ApiReturnType(int32_t)

        m3ApiGetArgMem(void*, i_ptr)
            m3ApiGetArg(int32_t, i_value)
                m3ApiGetArg(uint32_t, i_size)

                    uint32_t result = m3ApiPtrToOffset(memset(i_ptr, i_value, i_size));
    m3ApiReturn(result);
}

m3ApiRawFunction(m3_libc_malloc)
{
    m3ApiReturnType(int32_t);
    m3ApiGetArg(int32_t, i_size);
    // hack deal with overflow
    void* ptr = malloc(i_size);
    m3ApiReturn(m3ApiPtrToOffset(ptr))
}

TEST_F(WASM3Test, VMHeapAlloc)
{
    SWAModuleDescriptor module_desc;
    module_desc.name = "vm_heap_alloc";
    module_desc.wasm = vm_heap_alloc_wasm;
    module_desc.wasm_size = sizeof(vm_heap_alloc_wasm);
    module_desc.bytes_pinned_outside = true;
    SWAModuleId module = swa_create_module(runtime, &module_desc);
    EXPECT_NE(module, nullptr);
    swa::utilx::link(module, "*", "host_function", host_function_2);

    SWAHostFunctionDescriptor memset_func = {};
    memset_func.function_name = "memset";
    memset_func.module_name = "*";
    memset_func.signatures.m3 = "i(iii)";
    memset_func.backend_wrappers.m3 = &m3_libc_memset;
    swa_module_link_host_function(module, &memset_func);

    SWAHostFunctionDescriptor malloc_func = {};
    malloc_func.function_name = "malloc";
    malloc_func.module_name = "*";
    malloc_func.signatures.m3 = "i(i)";
    malloc_func.backend_wrappers.m3 = &m3_libc_malloc;
    swa_module_link_host_function(module, &malloc_func);

    auto ret = swa::utilx::executor(module, "heap_alloc").exec<int>();
    EXPECT_EQ(ret, 128 + 2);
}
