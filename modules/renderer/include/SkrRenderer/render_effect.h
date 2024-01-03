#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRenderer/primitive_pass.h"
#ifndef __meta__
    #include "SkrRenderer/render_effect.generated.h" // IWYU pragma: export
#endif

typedef const char8_t* skr_render_effect_name_t;
sreflect_struct(
    "guid" : "0a91d7fa-87f3-4dbb-a41a-45abd333e6ee", 
    "component" : { "buffer" : 4 } 
)
skr_render_effect_t
{
    skr_render_effect_name_t name;
    sugoi_entity_t effect_entity;
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct skr_render_effect_t skr_render_effect_t;
struct IRenderEffectProcessor;
struct VtblRenderEffectProcessor;

typedef sugoi_entity_t SGameEntity;
typedef sugoi_entity_t SRenderEffectEntity;

// Data operations for render effect
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_renderer_register_render_effect(SRendererId renderer, skr_render_effect_name_t name, IRenderEffectProcessor* processor);
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_renderer_register_render_effect_vtbl(SRendererId renderer, skr_render_effect_name_t name, VtblRenderEffectProcessor* processor);
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_renderer_remove_render_effect(SRendererId renderer, skr_render_effect_name_t name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_attach(SRendererId, sugoi_chunk_view_t* cv, skr_render_effect_name_t effect_name);
typedef void (*SProcRenderEffectAttach)(SRendererId, sugoi_chunk_view_t* cv, skr_render_effect_name_t effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_detach(SRendererId, sugoi_chunk_view_t* cv, skr_render_effect_name_t effect_name);
typedef void (*SProcRenderEffectDetach)(SRendererId, sugoi_chunk_view_t* cv, skr_render_effect_name_t effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_add_delta(SRendererId, sugoi_chunk_view_t* cv,
    skr_render_effect_name_t effect_name, sugoi_delta_type_t delta, sugoi_cast_callback_t callback, void* user_data);
typedef void (*SProcRenderEffectAddDelta)(SRendererId, sugoi_chunk_view_t* cv,
    skr_render_effect_name_t effect_name, sugoi_delta_type_t delta, sugoi_cast_callback_t callback, void* user_data);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_access(SRendererId, sugoi_chunk_view_t* cv,
    skr_render_effect_name_t effect_name, sugoi_view_callback_t view, void* u);
typedef void (*SProcRenderEffectAccess)(SRendererId, sugoi_chunk_view_t* cv,
    skr_render_effect_name_t effect_name, sugoi_view_callback_t view, void* u);

typedef struct skr_primitive_draw_context_t 
{
    SRendererId renderer;
    skr::render_graph::RenderGraph* render_graph;
    IPrimitiveRenderPass* pass;
    sugoi_storage_t* storage;
} skr_primitive_draw_context_t;

typedef struct skr_primitive_update_context_t 
{
    SRendererId renderer;
    skr::render_graph::RenderGraph* render_graph;
    sugoi_storage_t* storage;
} skr_primitive_update_context_t;

// Effect interfaces
typedef void (*SProcRenderEffectOnRegister)(SRendererId, sugoi_storage_t*);
typedef void (*SProcRenderEffectOnUnregister)(SRendererId, sugoi_storage_t*);
typedef void (*SProcRenderEffectGetTypeSet)(const sugoi_chunk_view_t* cv, sugoi_type_set_t* set);
typedef sugoi_type_index_t (*SProcRenderEffectGetIdentityType)();
typedef void (*SProcRenderEffectInitializeData)(SRendererId, sugoi_storage_t*, sugoi_chunk_view_t* game_cv, sugoi_chunk_view_t* render_cv);
// Drawcall interfaces for effect processor
typedef void (*SProcRenderEffectProduceDrawPackets)(const skr_primitive_draw_context_t* constext, skr_primitive_draw_packet_t* result);

typedef struct VtblRenderEffectProcessor {
    SProcRenderEffectOnRegister on_register;
    SProcRenderEffectOnUnregister on_unregister;

    SProcRenderEffectGetTypeSet get_type_set;
    SProcRenderEffectInitializeData initialize_data;
    SProcRenderEffectGetIdentityType get_identity_type;

    SProcRenderEffectProduceDrawPackets produce_draw_packets;
} VtblRenderEffectProcessor;

// render effect is a base class which iterates the world and produce drawcalls
typedef struct SKR_RENDERER_API IRenderEffectProcessor {
#ifdef __cplusplus
    virtual ~IRenderEffectProcessor() = default;

    virtual void on_register(SRendererId, sugoi_storage_t*) = 0;
    virtual void on_unregister(SRendererId, sugoi_storage_t*) = 0;

    virtual void get_type_set(const sugoi_chunk_view_t* cv, sugoi_type_set_t* set) = 0;
    virtual sugoi_type_index_t get_identity_type() = 0;
    virtual void initialize_data(SRendererId renderer, sugoi_storage_t* storage, sugoi_chunk_view_t* game_cv, sugoi_chunk_view_t* render_cv) = 0;

    virtual skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) = 0;

    virtual void on_update(const skr_primitive_update_context_t* context) {};
    virtual void post_update(const skr_primitive_update_context_t* context) {};
#endif
} IRenderEffectProcessor;

#ifdef __cplusplus
}
#endif