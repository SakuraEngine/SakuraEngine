#pragma once
#include "platform/configure.h"
#include "platform/guid.h"
#ifndef __meta__
    #include "GameRT/backend_config.generated.hpp"
#endif

typedef enum reflect attr(
"guid" : "b4b7f387-d8c2-465c-9b3a-6d83a3d198b1",
"serialize" : true
)
ECGPUBackEnd : uint8_t{ Vulkan, DX12, Metal } ECGPUBackEnd;

typedef struct reflect attr(
"guid" : "b537f7b1-6d2d-44f6-b313-bcb559d3f490",
"serialize" : true,
"config":true
)
config_backend_t
{
    enum ECGPUBackEnd backend;
}
config_backend_t;