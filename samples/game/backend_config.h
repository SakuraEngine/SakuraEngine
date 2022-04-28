#pragma once
#include "platform/configure.h"
#include "platform/guid.h"

typedef enum reflect attr(
"guid" : "b4b7f387-d8c2-465c-9b3a-6d83a3d198b1",
"asdsad" : true,
"fuck" : {
    "dddd" : [ true, false ]
}) ECGPUBackEnd{ Vulkan, DX12, Metal } ECGPUBackEnd;

typedef struct reflect attr("guid" : "b537f7b1-6d2d-44f6-b313-bcb559d3f490")
config_backend_t
{
    enum ECGPUBackEnd backend;
}
config_backend_t;

struct config_resource_header_t {
    skr_guid_t configType;
};