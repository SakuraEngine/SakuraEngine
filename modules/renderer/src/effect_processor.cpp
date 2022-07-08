#include "ecs/dual.h"
#include "ecs/array.hpp"
#include "skr_renderer/render_effects/effect_processor.h"
#include "skr_renderer/skr_renderer.h"
#include "EASTL/unordered_map.h"
#include "EASTL/vector.h"

struct SKR_RENDERER_API RenderEffectProcessorVtblProxy : public IRenderEffectProcessor {
    RenderEffectProcessorVtblProxy(VtblRenderEffectProcessor vtbl)
        : vtbl(vtbl)
    {
    }

    void get_type_set(const SGameEntity* entities, uint32_t count, dual_type_set_t* set) override
    {
        vtbl.get_type_set(entities, count, set);
    }

    void get_type_set_cv(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        vtbl.get_type_set_cv(cv, set);
    }

    uint32_t produce_drawcall(SGameSceneStorage* game_storage, SRenderStorage* effect_storage) override
    {
        return vtbl.produce_drawcall(game_storage, effect_storage);
    }

    void peek_drawcall(skr_primitive_draw_list_view_t* drawcalls) override
    {
        vtbl.peek_drawcall(drawcalls);
    }

    VtblRenderEffectProcessor vtbl;
};

struct SKR_RENDERER_API SkrRenderer : public skr::Renderer {
    friend class ::SkrRendererModule;

    ~SkrRenderer() override
    {
        for (auto proxy : processor_vtbl_proxies)
        {
            if (proxy) delete proxy;
        }
        processor_vtbl_proxies.clear();
        processors.clear();
    }

    eastl::unordered_map<const char*, IRenderEffectProcessor*> processors;
    eastl::vector<RenderEffectProcessorVtblProxy*> processor_vtbl_proxies;
};

void skr_renderer_register_render_effect(SkrRenderer* renderer, const char* name, IRenderEffectProcessor* processor)
{
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    renderer->processors[name] = processor;
}

void skr_renderer_register_render_effect_vtbl(SkrRenderer* renderer, const char* name, VtblRenderEffectProcessor* processor)
{
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    auto proxy = new RenderEffectProcessorVtblProxy(*processor);
    renderer->processor_vtbl_proxies.push_back(proxy);
    renderer->processors[name] = proxy;
}

SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();

void skr_render_effect_attach_cv(SkrRenderer* renderer, dual_chunk_view_t* cv, const char* effect_name)
{
    using render_effects_t = dual::array_component_T<skr_render_effect_t, 4>;
    auto feature_arrs = (render_effects_t*)dualV_get_owned_rw(cv, dual_id_of<skr_render_effect_t>::get());
    SKR_ASSERT(feature_arrs && "No render effect component in chunk view");
    if (feature_arrs)
    {
        auto world = skr_runtime_get_dual_storage();
        auto&& i_processor = renderer->processors.find(effect_name);

        if (i_processor == renderer->processors.end())
        {
            SKR_ASSERT(false && "No render effect processor registered");
            return;
        }
        dual_type_set_t type_set;
        i_processor->second->get_type_set_cv(cv, &type_set);
        for (uint32_t i = 0; i < cv->count; i++)
        {
            auto& features = feature_arrs[i];
            (void)features;
        }
    }
}