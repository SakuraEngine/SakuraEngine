#pragma once
#include "SkrRT/config.h"
#include "SkrRT/resource/resource_handle.h"
#include "SkrContainers/string.hpp"
#include "SkrContainers/variant.hpp"
#include "SkrContainers/sptr.hpp"
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/config.h"

#if !defined(__meta__)
    #include "RTTRTest/types.generated.h"
#endif

namespace Types sreflect
{
sreflect_enum("guid" : "1a0b91c7-6690-41d6-acfd-0c2b61f181f3")
sattr("serialize" : ["json", "bin"])
TestEnum : uint32_t
{
    Value0 = 0,
    Value1 = 1,
    Value2 = 2
};
} // namespace Types sreflect
