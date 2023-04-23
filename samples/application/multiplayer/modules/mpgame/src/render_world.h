#pragma once

#include "SkrRenderer/primitive_draw.h"
#include "ecs/dual.h"
#include "MPShared/client_world.h"
#include "SkrRenderGraph/api.h"
#include "ecs/type_builder.hpp"

struct MPRenderWorld
{
    dual_storage_t* storage;
    MPClientWorld* gameWorld;

    dual_query_t* renderGhostsQuery;
    dual_query_t* gameGhostsQuery;
    dual_query_t* transformQuery;
    dual_query_t* cameraQuery;
    skr::flat_hash_map<dual_entity_t, dual_entity_t> renderToGameEntityMap;
    skr::flat_hash_map<dual_entity_t, dual_entity_t> gameToRenderEntityMap;
    skr::vector<dual_entity_t> toDeleteRenderEntities;
    skr::vector<dual_entity_t> newGameEntities;
    SHiresTimer renderTimer;

    SRendererId renderer;

    double deltaTime = 0.0;

    void Initialize(MPClientWorld* gameWorld);
    void Shutdown();
    void UpdateStructuralChanges();
    void Update();
    void Render();
    void LoadScene();
    dual::type_builder_t GetRenderEntityType(skr_resource_handle_t prefab, bool controller);
};