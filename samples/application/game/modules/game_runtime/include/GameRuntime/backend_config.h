#pragma once
#include "GameRuntime/module.configure.h"
#include "resource/config_resource.h"
#ifndef __meta__
    #include "GameRuntime/backend_config.generated.h"
#endif

sreflect_enum("guid" : "b4b7f387-d8c2-465c-9b3a-6d83a3d198b1")
sattr("serialize" : ["json", "bin"])
sattr("rtti" : true)
ECGPUBackEnd SKRENUM(uint32_t){
    Vulkan,
    DX12, 
    Metal
};
typedef enum ECGPUBackEnd ECGPUBackEnd;

sreflect_struct("guid" : "b537f7b1-6d2d-44f6-b313-bcb559d3f490")
config_backend_t
{
    ECGPUBackEnd backend;
}
sattr("serialize" : ["json", "bin"])
sattr("rtti" : true);
typedef struct config_backend_t config_backend_t;
