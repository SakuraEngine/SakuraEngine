#pragma once
#include "platform/configure.h"
#ifndef __meta__
    #include "GameRT/backend_config2.generated.h"
#endif

enum sreflect 
sattr("guid" : "9688e3fe-72eb-4c8d-a6f2-0a38585fbe26")
ECGPUBackEnd2 : uint32_t
{
    Vulkan_2, 
    DX12_2, 
    Metal_2 
};
