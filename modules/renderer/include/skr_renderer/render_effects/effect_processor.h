#pragma once
#include "skr_renderer/skr_renderer_config.h"
#include "ecs/dualX.h"
#include "skr_renderer/primitive_draw.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sreflect sattr(
    "guid" : "0a91d7fa-87f3-4dbb-a41a-45abd333e6ee",
    "component" : 
    {
        "buffer" : 4
    }
) skr_render_effect_t
{
    dual_entity_t entity;
};
typedef struct skr_render_effect_t skr_render_effect_t;

typedef dual_entity_t SGameEntity;
typedef dual_entity_t SRenderEffectEntity;
typedef dual_storage_t SGameSceneStorage;
typedef dual_storage_t SRenderStorage;

// Effect interfaces
typedef void (*SProcRenderEffectOnAttach)(SGameEntity entity, SRenderEffectEntity effect);
typedef void (*SProcRenderEffectOnDetach)(SGameEntity entity, SRenderEffectEntity effect);

// Data operations for render effect
typedef void (*SProcRenderEffectAttach)(SGameEntity entity, const char* effect_name);
typedef void (*SProcRenderEffectDetach)(SGameEntity entity, const char* effect_name);
typedef void (*SProcRenderEffectQuery)(const char* effect_name, dual_view_callback_t callback, void* user_data);
typedef void (*SProcRenderEffectAccess)(SGameEntity entity, const char* effect_name, dual_chunk_view_t* view);
typedef void (*SProcRenderEffectAddData)(SGameEntity entity, const char* effect_name, dual_type_index_t type);
typedef void (*SProcRenderEffectRemoveData)(SGameEntity entity, const char* effect_name, dual_type_index_t type);

// Drawcall interfaces for effect processor
typedef uint32_t (*SProcRenderEffectProduceDrawcall)(SGameSceneStorage game_storage, SRenderStorage effect_storage);
typedef void (*SProcRenderEffectPeekDrawcall)(skr_primitive_draw_t* drawcalls, uint32_t peek_count);

typedef struct VtblRenderEffectProcessor {
    SProcRenderEffectOnAttach on_attach;
    SProcRenderEffectOnDetach on_detach;
    SProcRenderEffectProduceDrawcall produce_drawcall;
    SProcRenderEffectPeekDrawcall peek_drawcall;
} VtblRenderEffectProcessor;

#ifdef __cplusplus
}

typedef struct IRenderEffectProcessor {
    virtual void on_attach(SGameEntity entity, SRenderEffectEntity effect) = 0;
    virtual void on_detach(SGameEntity entity, SRenderEffectEntity effect) = 0;
    virtual uint32_t produce_drawcall(SGameSceneStorage game_storage, SRenderStorage effect_storage) = 0;
    virtual void peek_drawcall(skr_primitive_draw_t* drawcalls, uint32_t peek_count) = 0;
} IRenderEffectProcessor;

#endif