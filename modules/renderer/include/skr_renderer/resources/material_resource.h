#pragma once
#include "SkrRenderer/module.configure.h"
#include "skr_renderer/fwd_types.h"
#include "resource/resource_handle.h"
#ifndef __meta__
    #include "SkrRenderer/resources/material_resource.generated.h"
#endif

sreflect_struct("guid" : "83264b35-3fde-4fff-8ee1-89abce2e445b")
sattr("serialize" : ["json", "bin"])
skr_material_type_resource_t
{
    uint32_t version;
    skr_resource_handle_t shader_pipeline;   // ?
};
typedef struct skr_material_type_resource_t skr_material_type_resource_t;

sreflect_struct("guid" : "2efad635-b331-4fc6-8c52-2f8ca954823e")
sattr("serialize" : ["json", "bin"])
skr_material_resource_t
{
    skr_resource_handle_t material_type;
    uint32_t material_type_version;
};
typedef struct skr_material_resource_t skr_material_resource_t;