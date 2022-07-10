#include "utils/make_zeroed.hpp"
#include "ecs/dual.h"
#include "ecs/array.hpp"
#include "ecs/callback.hpp"
#include "skr_renderer/effect_processor.h"
#include "skr_renderer/skr_renderer.h"
#include "EASTL/unordered_map.h"
#include "EASTL/vector.h"

SKR_IMPORT_API struct dual_storage_t* skr_runtime_get_dual_storage();

struct SKR_RENDERER_API RenderEffectProcessorVtblProxy : public IRenderEffectProcessor {
    RenderEffectProcessorVtblProxy(VtblRenderEffectProcessor vtbl)
        : vtbl(vtbl)
    {
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        if (vtbl.get_type_set)
            vtbl.get_type_set(cv, set);
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* cv) override
    {
        if (vtbl.initialize_data)
            vtbl.initialize_data(renderer, storage, cv);
    }

    uint32_t produce_drawcall(IPrimitiveRenderPass* pass, dual_storage_t* game_storage) override
    {
        if (vtbl.produce_drawcall)
            return vtbl.produce_drawcall(pass, game_storage);
        return 0;
    }

    void peek_drawcall(IPrimitiveRenderPass* pass, skr_primitive_draw_list_view_t* drawcalls) override
    {
        if (vtbl.peek_drawcall)
            vtbl.peek_drawcall(pass, drawcalls);
    }

    VtblRenderEffectProcessor vtbl;
};

struct SKR_RENDERER_API SkrRendererImpl : public skr::Renderer {
    friend class ::SkrRendererModule;

    ~SkrRendererImpl() override
    {
        for (auto proxy : processor_vtbl_proxies)
        {
            if (proxy) delete proxy;
        }
        processor_vtbl_proxies.clear();
        processors.clear();
    }

    void render(skr::render_graph::RenderGraph* render_graph) override
    {
        // produce draw calls
        for (auto& pass : passes)
        {
            auto& drawcall_arena = drawcalls[pass.second->identity()];
            // used out
            drawcall_arena.clear();
            for (auto& processor : processors)
            {
                auto storage = skr_runtime_get_dual_storage();
                auto dcn = processor.second->produce_drawcall(pass.second, storage);
                drawcall_arena.resize(dcn + drawcall_arena.size());
                if (dcn && pass.second && processor.second)
                {
                    auto drawcalls = make_zeroed<skr_primitive_draw_list_view_t>();
                    drawcalls.drawcalls = drawcall_arena.data();
                    drawcalls.count = dcn;
                    processor.second->peek_drawcall(pass.second, &drawcalls);
                }
            }
        }
    }

    eastl::vector_map<eastl::string, eastl::vector<skr_primitive_draw_t>> drawcalls;
    eastl::unordered_map<eastl::string, IPrimitiveRenderPass*> passes;
    eastl::unordered_map<eastl::string, IRenderEffectProcessor*> processors;
    eastl::vector<RenderEffectProcessorVtblProxy*> processor_vtbl_proxies;
};

skr::Renderer* create_renderer_impl()
{
    return new SkrRendererImpl();
}

void skr_renderer_register_render_pass(ISkrRenderer* r, skr_render_effect_name_t name, IPrimitiveRenderPass* pass)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->passes.find(name); _ != renderer->passes.end())
    {
        SKR_ASSERT(false && "Render pass already registered");
        return;
    }
    renderer->passes[name] = pass;
}

void skr_renderer_remove_render_pass(ISkrRenderer* r, skr_render_pass_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->passes.find(name); _ != renderer->passes.end())
    {
        renderer->passes.erase(_);
    }
}

void skr_renderer_register_render_effect(ISkrRenderer* r, skr_render_effect_name_t name, IRenderEffectProcessor* processor)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    renderer->processors[name] = processor;
}

void skr_renderer_register_render_effect_vtbl(ISkrRenderer* r, skr_render_effect_name_t name, VtblRenderEffectProcessor* processor)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    auto proxy = new RenderEffectProcessorVtblProxy(*processor);
    renderer->processor_vtbl_proxies.push_back(proxy);
    renderer->processors[name] = proxy;
}

void skr_renderer_remove_render_effect(ISkrRenderer* r, skr_render_effect_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        renderer->processors.erase(_);
    }
}

using render_effects_t = dual::array_component_T<skr_render_effect_t, 4>;

void skr_render_effect_attach(ISkrRenderer* r, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name)
{
    auto renderer = (SkrRendererImpl*)r;
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
        auto entity_type = make_zeroed<dual_entity_type_t>();
        i_processor->second->get_type_set(cv, &entity_type.type);
        // create render effect entities in storage
        dual_chunk_view_t* out_cv = nullptr;
        auto initialize_callback = [&](dual_chunk_view_t* inView) {
            // do user initialize callback
            i_processor->second->initialize_data(renderer, world, cv);
            out_cv = inView;
        };
        dualS_allocate_type(world, &entity_type, cv->count, DUAL_LAMBDA(initialize_callback));
        // attach render effect entities to game entities
        if (out_cv)
        {
            auto entities = dualV_get_entities(out_cv);
            for (uint32_t i = 0; i < cv->count; i++)
            {
                auto& features = feature_arrs[i];
#ifdef _DEBUG
                for (auto& _ : features)
                {
                    SKR_ASSERT(strcmp(_.name, effect_name) != 0 && "Render effect already attached");
                }
#endif
                features.push_back({ nullptr, NULL_ENTITY });
                auto& feature = features.back();
                feature.name = effect_name;
                feature.effect_entity = entities[i];
            }
        }
    }
}

void skr_render_effect_detach(ISkrRenderer* r, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name)
{
    if (cv)
    {
        auto feature_arrs = (render_effects_t*)dualV_get_owned_rw(cv, dual_id_of<skr_render_effect_t>::get());
        if (feature_arrs)
        {
            uint32_t removed_index = 0;
            for (uint32_t i = 0; i < cv->count; i++)
            {
                auto& features = feature_arrs[i];
                for (auto& _ : features)
                {
                    if (strcmp(_.name, effect_name) == 0)
                    {
                        _.name = nullptr;
                        _.effect_entity = NULL_ENTITY;
                        removed_index = i;
                    }
                }
                features.erase(features.begin() + removed_index);
            }
        }
    }
}

void skr_render_effect_add_delta(ISkrRenderer* r, const SGameEntity* entities, uint32_t count,
skr_render_effect_name_t effect_name, dual_delta_type_t delta,
dual_cast_callback_t callback, void* user_data)
{
    if (count)
    {
        auto storage = skr_runtime_get_dual_storage();
        SKR_ASSERT(storage && "No dual storage");
        eastl::vector<dual_entity_t> render_effects;
        render_effects.reserve(count);
        // batch game ents to collect render effects
        auto game_batch_callback = [&](dual_chunk_view_t* view) {
            auto feature_arrs = (render_effects_t*)dualV_get_owned_ro(view, dual_id_of<skr_render_effect_t>::get());
            if (feature_arrs)
            {
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto& features = feature_arrs[i];
                    for (auto& _ : features)
                    {
                        if (strcmp(_.name, effect_name) == 0)
                        {
                            render_effects.emplace_back(_.effect_entity);
                        }
                    }
                }
            }
        };
        dualS_batch(storage, entities, count, DUAL_LAMBDA(game_batch_callback));
        // do cast for render effects
        auto render_batch_callback = [&](dual_chunk_view_t* view) {
            dualS_cast_view_delta(storage, view, &delta, callback, user_data);
        };
        dualS_batch(storage, render_effects.data(), (EIndex)render_effects.size(), DUAL_LAMBDA(render_batch_callback));
    }
}

void skr_render_effect_access(ISkrRenderer* r, const SGameEntity* entities, uint32_t count, skr_render_effect_name_t effect_name, dual_view_callback_t view, void* u)
{
    if (count)
    {
        auto storage = skr_runtime_get_dual_storage();
        SKR_ASSERT(storage && "No dual storage");
        eastl::vector<dual_entity_t> render_effects;
        render_effects.reserve(count);
        // batch game ents to collect render effects
        auto game_batch_callback = [&](dual_chunk_view_t* view) {
            auto feature_arrs = (render_effects_t*)dualV_get_owned_ro(view, dual_id_of<skr_render_effect_t>::get());
            if (feature_arrs)
            {
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto& features = feature_arrs[i];
                    for (auto& _ : features)
                    {
                        if (strcmp(_.name, effect_name) == 0)
                        {
                            render_effects.emplace_back(_.effect_entity);
                        }
                    }
                }
            }
        };
        dualS_batch(storage, entities, count, DUAL_LAMBDA(game_batch_callback));
        // do cast for render effects
        dualS_batch(storage, render_effects.data(), (EIndex)render_effects.size(), view, u);
    }
}
