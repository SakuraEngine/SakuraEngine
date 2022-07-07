#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "ecs/dual.h"
#include "ecs/dualX.h"
#include "skr_renderer/primitive_draw.h"

struct sreflect sattr(
    "guid" : "0a91d7fa-87f3-4dbb-a41a-45abd333e6ee",
    "component" : 
    {
        "buffer" : 4
    }
) skr_render_effect_t
{
    dual_entity_t entity;
    const char* name;
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct skr_render_effect_t skr_render_effect_t;
struct IRenderEffectProcessor;
struct VtblRenderEffectProcessor;

typedef dual_entity_t SGameEntity;
typedef dual_entity_t SRenderEffectEntity;
typedef dual_storage_t SGameSceneStorage;
typedef dual_storage_t SRenderStorage;
typedef struct SkrRenderer SkrRenderer;

// Data operations for render effect
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_renderer_register_render_effect(SkrRenderer* renderer, const char* name, IRenderEffectProcessor* processor);
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_renderer_register_render_effect_vtbl(SkrRenderer* renderer, const char* name, VtblRenderEffectProcessor* processor);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_attach(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name);
typedef void (*SProcRenderEffectAttach)(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_detach(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name);
typedef void (*SProcRenderEffectDetach)(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_attach_cv(SkrRenderer*, dual_chunk_view_t* cv, const char* effect_name);
typedef void (*SProcRenderEffectAttachCV)(SkrRenderer*, dual_chunk_view_t* cv, const char* effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_detach_cv(SkrRenderer*, dual_chunk_view_t* cv, const char* effect_name);
typedef void (*SProcRenderEffectDetachCV)(SkrRenderer*, dual_chunk_view_t* cv, const char* effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_query(SkrRenderer*, const char* effect_name, dual_view_callback_t callback, void* user_data);
typedef void (*SProcRenderEffectQuery)(SkrRenderer*, const char* effect_name, dual_view_callback_t callback, void* user_data);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_access(SkrRenderer*, SGameEntity entity, const char* effect_name, dual_chunk_view_t* view);
typedef void (*SProcRenderEffectAccess)(SkrRenderer*, SGameEntity entity, const char* effect_name, dual_chunk_view_t* view);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_add_data(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name, dual_type_index_t type);
typedef void (*SProcRenderEffectAddData)(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name, dual_type_index_t type);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_effect_remove_data(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name, dual_type_index_t type);
typedef void (*SProcRenderEffectRemoveData)(SkrRenderer*, const SGameEntity* entities, uint32_t count, const char* effect_name, dual_type_index_t type);

// Effect interfaces
// typedef void (*SProcRenderEffectOnAttach)(const SGameEntity* entities, uint32_t count, SRenderEffectEntity effect);
// typedef void (*SProcRenderEffectOnDetach)(const SGameEntity* entities, uint32_t count, SRenderEffectEntity effect);
typedef void (*SProcRenderEffectGetTypeSet)(const SGameEntity* entities, uint32_t count, dual_type_set_t* set);
typedef void (*SProcRenderEffectGetTypeSetCV)(const dual_chunk_view_t* cv, dual_type_set_t* set);
// Drawcall interfaces for effect processor
typedef uint32_t (*SProcRenderEffectProduceDrawcall)(SGameSceneStorage* game_storage, SRenderStorage* effect_storage);
typedef void (*SProcRenderEffectPeekDrawcall)(skr_primitive_draw_list_view_t* drawcalls);

typedef struct VtblRenderEffectProcessor {
    SProcRenderEffectGetTypeSet get_type_set;
    SProcRenderEffectGetTypeSetCV get_type_set_cv;

    SProcRenderEffectProduceDrawcall produce_drawcall;
    SProcRenderEffectPeekDrawcall peek_drawcall;
} VtblRenderEffectProcessor;

typedef struct SKR_RENDERER_API IRenderEffectProcessor {
#ifdef __cplusplus
    virtual ~IRenderEffectProcessor() = default;

    virtual void get_type_set(const SGameEntity* entities, uint32_t count, dual_type_set_t* set) = 0;
    virtual void get_type_set_cv(const dual_chunk_view_t* cv, dual_type_set_t* set) = 0;

    virtual uint32_t produce_drawcall(SGameSceneStorage* game_storage, SRenderStorage* effect_storage) = 0;
    virtual void peek_drawcall(skr_primitive_draw_list_view_t* drawcalls) = 0;

#endif
} IRenderEffectProcessor;

#ifdef __cplusplus
}
#endif