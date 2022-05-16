#pragma once
#include "ecs/dual.h"
// align up & can directly update with GPU-scene
#include "scene.h"
#include "cgpu/api.h"

// root signatures & pipeline-objects are hidden bardward.

typedef struct pipeline_shaders_t {
    CGPUShaderLibraryId vs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId hs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId ds SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId gs SKR_IF_CPP(= nullptr);
    CGPUShaderLibraryId ps SKR_IF_CPP(= nullptr);
} pipeline_shaders_t;
typedef dual_entity_t pipeline_shaders_id_t;

typedef struct processor_shaders_t {
    CGPUShaderLibraryId cs SKR_IF_CPP(= nullptr);
} processor_shaders_t;
typedef dual_entity_t processor_shaders_id_t;

typedef dual_entity_t material_id_t;