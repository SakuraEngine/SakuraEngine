#pragma once
// TODO. 移动到专门的 test 文件
#include "SkrRT/config.h"
#include "SkrRTTR/iobject.hpp"
#if !defined(__meta__)
    #include "test_serde_types.generated.h"
#endif

namespace test_serde
{
sreflect_struct(
    guid = "a3847f6a-05b0-4049-a619-e687ef4bc856"
    serde = @json
)
TestJson {
    SKR_GENERATE_BODY()

    // in json: {"serde_normal": <number>}
    int32_t serde_normal;
    
    // in json: {"class": <number>}
    sattr(serde.alias = "class")
    int32_t klass;

    // will not appare in json
    sattr(serde = @disable)
    int32_t disabled;

private:
    int32_t _private_member;
};

sreflect_struct(
    guid = "eb2a94ea-e0f4-4b93-9d3f-17d13ea3af14"
    serde = @bin
)
TestBin {
    SKR_GENERATE_BODY()

    // in json: {"serde_normal": <number>}
    int32_t serde_normal;

    // will not appare in bin
    sattr(serde = @disable)
    int32_t disabled;

private:
    int32_t _private_member;
};
} // namespace test_serde