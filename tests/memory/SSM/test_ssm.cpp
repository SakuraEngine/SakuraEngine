#include "SkrCore/memory/memory.h"
#include "SkrCore/memory/sysmem_pool.h"
#include "SkrTestFramework/framework.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

struct MMapTests {
protected:
    MMapTests() {}
    ~MMapTests() {}
};

TEST_CASE_METHOD(MMapTests, "malloc/free")
{
    auto p = sakura_malloc(64);
    EXPECT_NE(p, nullptr);
    sakura_free(p);
}

TEST_CASE_METHOD(MMapTests, "sysmem_pool")
{
    {
        SSysMemoryPoolDesc pool_desc = {};
        pool_desc.pool_name = "sysmem_pool";
        pool_desc.size = 1024;
        pool_desc.use_large_page = false;

        auto sysmem_pool = sakura_sysmem_pool_create(&pool_desc);
        EXPECT_NE(sysmem_pool, nullptr);
        auto p = (uint8_t*)sakura_sysmem_pool_malloc(sysmem_pool, 64);
        EXPECT_NE(p, nullptr);
        p[63] = 2;
        EXPECT_EQ(p[63], 2);
        sakura_sysmem_pool_free(sysmem_pool, p);
        sakura_sysmem_pool_destroy(sysmem_pool);
    }
    {
        SSysMemoryPoolDesc pool_desc = {};
        pool_desc.pool_name = "sysmem_pool";
        pool_desc.size = 2 * 1024 * 1024;
        pool_desc.use_large_page = true;

        auto sysmem_pool = sakura_sysmem_pool_create(&pool_desc);
        EXPECT_NE(sysmem_pool, nullptr);
        auto p = (uint8_t*)sakura_sysmem_pool_malloc(sysmem_pool, 64);
        EXPECT_NE(p, nullptr);
        p[63] = 2;
        EXPECT_EQ(p[63], 2);
        sakura_sysmem_pool_free(sysmem_pool, p);
        sakura_sysmem_pool_destroy(sysmem_pool);
    }
}