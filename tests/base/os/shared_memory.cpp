#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrTestFramework/framework.hpp"
#include "SkrOS/shared_memory.hpp"

struct SHARED_MEMORY_TEST
{

};

using namespace skr::literals;
static constexpr auto kID = u8"bfbeb1c5-bca1-4743-bc1e-a4ca1ba78fd4"_guid;
TEST_CASE_METHOD(SHARED_MEMORY_TEST, "sm")
{
    skr::SharedMemory sm(kID, 4096);
    auto addr = sm.address();
    CHECK(addr != nullptr);
    memcpy(addr, "Hello, World!", 14);

    skr::SharedMemory sm2(kID, 4096);
    auto addr2 = sm2.address();
    CHECK(addr2 != nullptr);
    CHECK(memcmp(addr2, "Hello, World!", 14) == 0);
}