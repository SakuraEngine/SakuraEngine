#pragma once

#include "SkrRenderer/primitive_draw.h"
#include "SkrRT/ecs/sugoi.h"
#include "MPShared/client_world.h"
#include "SkrRenderGraph/api.h"
#include "SkrRT/ecs/type_builder.hpp"

struct MPRenderWorld
{
    sugoi_storage_t* storage;
    MPClientWorld* gameWorld;

    sugoi_query_t* renderGhostsQuery;
    sugoi_query_t* gameGhostsQuery;
    sugoi_query_t* transformQuery;
    sugoi_query_t* cameraQuery;
    skr::FlatHashMap<sugoi_entity_t, sugoi_entity_t> renderToGameEntityMap;
    skr::FlatHashMap<sugoi_entity_t, sugoi_entity_t> gameToRenderEntityMap;
    skr::Vector<sugoi_entity_t> toDeleteRenderEntities;
    skr::Vector<sugoi_entity_t> newGameEntities;
    SHiresTimer renderTimer;

    SRendererId renderer;

    double deltaTime = 0.0;

    void Initialize(MPClientWorld* gameWorld);
    void Shutdown();
    void UpdateStructuralChanges();
    void Update();
    void Render();
    void LoadScene();
    sugoi::TypeSetBuilder GetRenderEntityType(skr_resource_handle_t prefab, bool controller);
};