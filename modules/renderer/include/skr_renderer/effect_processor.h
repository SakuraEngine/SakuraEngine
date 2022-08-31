#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "ecs/dual.h"
#include "ecs/dualX.h"
#include "skr_renderer/primitive_pass.h"

typedef const char* skr_render_effect_name_t;
struct sreflect sattr(
    "guid" : "0a91d7fa-87f3-4dbb-a41a-45abd333e6ee",
    "component" : 
    {
        "buffer" : 4
    }
) skr_render_effect_t
{
    skr_render_effect_name_t name;
    dual_entity_t effect_entity;
};

#ifdef __cplusplus
extern "C" {
#endif

typedef struct skr_render_effect_t skr_render_effect_t;
struct IRenderEffectProcessor;
struct VtblRenderEffectProcessor;

typedef dual_entity_t SGameEntity;
typedef dual_entity_t SRenderEffectEntity;

// Data operations for render effect
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_renderer_register_render_effect(ISkrRenderer* renderer, skr_render_effect_name_t name, IRenderEffectProcessor* processor);
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_renderer_register_render_effect_vtbl(ISkrRenderer* renderer, skr_render_effect_name_t name, VtblRenderEffectProcessor* processor);
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_renderer_remove_render_effect(ISkrRenderer* renderer, skr_render_effect_name_t name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_attach(ISkrRenderer*, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name);
typedef void (*SProcRenderEffectAttach)(ISkrRenderer*, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_detach(ISkrRenderer*, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name);
typedef void (*SProcRenderEffectDetach)(ISkrRenderer*, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_add_delta(ISkrRenderer*, const SGameEntity* entities, uint32_t count,
skr_render_effect_name_t effect_name, dual_delta_type_t delta, dual_cast_callback_t callback, void* user_data);
typedef void (*SProcRenderEffectAddDelta)(ISkrRenderer*, const SGameEntity* entities, uint32_t count,
skr_render_effect_name_t effect_name, dual_delta_type_t delta, dual_cast_callback_t callback, void* user_data);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void skr_render_effect_access(ISkrRenderer*, const SGameEntity* entities, uint32_t count,
skr_render_effect_name_t effect_name, dual_view_callback_t view, void* u);
typedef void (*SProcRenderEffectAccess)(ISkrRenderer*, const SGameEntity* entities, uint32_t count,
skr_render_effect_name_t effect_name, dual_view_callback_t view, void* u);

// Effect interfaces
typedef void (*SProcRenderEffectOnRegister)(ISkrRenderer*, dual_storage_t*);
typedef void (*SProcRenderEffectOnUnregister)(ISkrRenderer*, dual_storage_t*);
typedef void (*SProcRenderEffectGetTypeSet)(const dual_chunk_view_t* cv, dual_type_set_t* set);
typedef dual_type_index_t (*SProcRenderEffectGetIdentityType)();
typedef void (*SProcRenderEffectInitializeData)(ISkrRenderer*, dual_storage_t*, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv);
// Drawcall interfaces for effect processor
typedef void (*SProcRenderEffectProduceDrawPackets)(IPrimitiveRenderPass* pass, dual_storage_t* storage, skr_primitive_draw_packet_t* result);

typedef struct VtblRenderEffectProcessor {
    SProcRenderEffectOnRegister on_register;
    SProcRenderEffectOnUnregister on_unregister;

    SProcRenderEffectGetTypeSet get_type_set;
    SProcRenderEffectInitializeData initialize_data;
    SProcRenderEffectGetIdentityType get_identity_type;

    SProcRenderEffectProduceDrawPackets produce_draw_packets;
} VtblRenderEffectProcessor;

typedef struct SKR_RENDERER_API IRenderEffectProcessor {
#ifdef __cplusplus
    virtual ~IRenderEffectProcessor() = default;

    virtual void on_register(ISkrRenderer*, dual_storage_t*) = 0;
    virtual void on_unregister(ISkrRenderer*, dual_storage_t*) = 0;

    virtual void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) = 0;
    virtual dual_type_index_t get_identity_type() = 0;
    virtual void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) = 0;

    virtual skr_primitive_draw_packet_t produce_draw_packets(IPrimitiveRenderPass* pass, dual_storage_t* storage) = 0;
#endif
} IRenderEffectProcessor;

#ifdef __cplusplus
}
#endif