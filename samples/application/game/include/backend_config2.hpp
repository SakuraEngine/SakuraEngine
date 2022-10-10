#pragma once
#include "platform/configure.h"
#include "utils/types.h"

struct SRuntimeAttribute* CreateAttribute();

enum sreflect
sattr("guid" : "b4b7f387-d8c2-465c-9b3a-6d83a3d198b1")
sattr("asdsad" : true)
sattr("fuck" : { "dddd" : [ true, false ] }) 
sruntime_attr(0, CreateAttribute())
ECGPUBackEnd2 : uint32_t
{
    Vulkan_2, 
    DX12_2, 
    Metal_2 
};
