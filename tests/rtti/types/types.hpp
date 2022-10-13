#pragma once
#include "platform/configure.h"
#include "utils/types.h"

struct SRuntimeAttribute* CreateAttribute();

namespace Types sreflect
{

enum sreflect
sattr(
    "guid" : "1a0b91c7-6690-41d6-acfd-0c2b61f181f3",
    "rtti" : true
)
sruntime_attr(0, CreateAttribute())
TestEnum : uint32_t
{
    Value0 = 0, 
    Value1 = 1, 
    Value2 = 2 
};

}