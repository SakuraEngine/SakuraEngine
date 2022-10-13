#pragma once
#include "platform/configure.h"
#include "utils/types.h"

struct SRuntimeAttribute* CreateAttribute();

enum sreflect
sattr("guid" : "9688e3fe-72eb-4c8d-a6f2-0a38585fbe26")
sattr("asdsad" : true)
sattr("fuck" : { "dddd" : [ true, false ] }) 
sruntime_attr(0, CreateAttribute())
ECGPUBackEnd2 : uint32_t
{
    Vulkan_2, 
    DX12_2, 
    Metal_2 
};
