#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrTestFramework/framework.hpp"
#include "SkrOS/shared_memory.hpp"

struct SHARED_MEMORY
{

};

TEST_CASE_METHOD(SHARED_MEMORY, "sm")
{
    skr::SharedMemory sm(114514, 4096);
    auto addr = sm.address();
    CHECK(addr != nullptr);
    memcpy(addr, "Hello, World!", 14);

    skr::SharedMemory sm2(114514, 4096);
    auto addr2 = sm2.address();
    CHECK(addr2 != nullptr);
    CHECK(memcmp(addr, addr2, 14) == 0);
}