#pragma once
#include "platform/configure.h"
#include "platform/guid.h"
#ifndef __meta__
    #include "GameRT/backend_config2.generated.hpp"
#endif

typedef enum reflect attr(
"guid" : "b4b7f387-d8c2-465c-9b3a-6d83a3d198b1",
"asdsad" : true,
"fuck" : {
    "dddd" : [ true, false ]
}) ECGPUBackEnd2 : uint8_t{ Vulkan_2, DX12_2, Metal_2 } ECGPUBackEnd2;