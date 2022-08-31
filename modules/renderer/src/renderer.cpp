#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "ecs/dual.h"
#include "ecs/array.hpp"
#include "ecs/callback.hpp"
#include "render_graph/frontend/render_graph.hpp"
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

    void on_register(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        if (vtbl.on_register)
            vtbl.on_register(renderer, storage);
    }

    void on_unregister(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        if (vtbl.on_unregister)
            vtbl.on_unregister(renderer, storage);
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        if (vtbl.get_type_set)
            vtbl.get_type_set(cv, set);
    }

    dual_type_index_t get_identity_type() override
    {
        if (vtbl.get_identity_type)
            return vtbl.get_identity_type();
        return DUAL_NULL_TYPE;
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {
        if (vtbl.initialize_data)
            vtbl.initialize_data(renderer, storage, game_cv, render_cv);
    }

    skr_primitive_draw_packet_t produce_draw_packets(IPrimitiveRenderPass* pass, dual_storage_t* storage) override
    {
        skr_primitive_draw_packet_t result = {};
        if (vtbl.produce_draw_packets)
            vtbl.produce_draw_packets(pass, storage, &result);
        return result;
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

    void render(skr::render_graph::RenderGraph* render_graph, dual_storage_t* storage) override
    {
        // produce draw calls
        for (auto& pass : passes)
        {
            draw_packets[pass.second->identity()].clear();
            // used out
            for (auto& processor : processors)
            {
                if (pass.second && processor.second)
                {
                    auto packet = processor.second->produce_draw_packets(pass.second, storage);
                    draw_packets[pass.second->identity()].emplace_back(packet);
                }
            }
        }
        // execute draw calls
        for (auto& pass : passes)
        {
            if (pass.second)
            {
                auto& pass_draw_packets = draw_packets[pass.second->identity()];
                for (auto pass_draw_packet : pass_draw_packets)
                {
                    for (uint32_t i = 0; i < pass_draw_packet.count; i++)
                    {
                        pass.second->execute(render_graph, pass_draw_packet.lists[i]);
                    }
                }
            }
        }
    }

    eastl::vector_map<eastl::string, IPrimitiveRenderPass*> passes;
    eastl::vector_map<eastl::string, IRenderEffectProcessor*> processors;
    eastl::vector<RenderEffectProcessorVtblProxy*> processor_vtbl_proxies;
protected:
    eastl::vector_map<eastl::string, eastl::vector<skr_primitive_draw_packet_t>> draw_packets;
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
    pass->on_register(r);
}

void skr_renderer_remove_render_pass(ISkrRenderer* r, skr_render_pass_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->passes.find(name); _ != renderer->passes.end())
    {
        _->second->on_unregister(r);
        renderer->passes.erase(_);
    }
}

void skr_renderer_register_render_effect(ISkrRenderer* r, skr_render_effect_name_t name, IRenderEffectProcessor* processor)
{
    auto storage = skr_runtime_get_dual_storage();
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    renderer->processors[name] = processor;
    processor->on_register(r, storage);
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
    auto storage = skr_runtime_get_dual_storage();
    if (auto&& _ = renderer->processors.find(name); _ != renderer->processors.end())
    {
        _->second->on_unregister(r, storage);
        renderer->processors.erase(_);
    }
}

using render_effects_t = dual::array_component_T<skr_render_effect_t, 4>;

void skr_render_effect_attach(ISkrRenderer* r, dual_chunk_view_t* g_cv, skr_render_effect_name_t effect_name)
{
    auto renderer = (SkrRendererImpl*)r;
    auto feature_arrs = (render_effects_t*)dualV_get_owned_rw(g_cv, dual_id_of<skr_render_effect_t>::get());
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
        i_processor->second->get_type_set(g_cv, &entity_type.type);
        // create render effect entities in storage
        if (entity_type.type.length != 0)
        {
            dual_chunk_view_t* out_cv = nullptr;
            auto initialize_callback = [&](dual_chunk_view_t* r_cv) {
                // do user initialize callback
                i_processor->second->initialize_data(renderer, world, g_cv, r_cv);
                out_cv = r_cv;
            };
            dualS_allocate_type(world, &entity_type, g_cv->count, DUAL_LAMBDA(initialize_callback));
            // attach render effect entities to game entities
            if (out_cv)
            {
                auto entities = dualV_get_entities(out_cv);
                for (uint32_t i = 0; i < g_cv->count; i++)
                {
                    auto& features = feature_arrs[i];
#ifdef _DEBUG
                    for (auto& _ : features)
                    {
                        SKR_ASSERT(strcmp(_.name, effect_name) != 0 && "Render effect already attached");
                    }
#endif
                    features.push_back({ nullptr, DUAL_NULL_ENTITY });
                    auto& feature = features.back();
                    feature.name = effect_name;
                    feature.effect_entity = entities[i];
                }
            }
        }
        else
        {
            SKR_LOG_WARN("Render Effect %s privided no valid component types! At least identity type should be provided!", effect_name);
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
                        _.effect_entity = DUAL_NULL_ENTITY;
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
        eastl::vector<dual_entity_t> batch_render_effects;
        batch_render_effects.reserve(count);
        // batch game ents to collect render effects
        auto game_batch_callback = [&](dual_chunk_view_t* view) {
            auto effects_chunk = (render_effects_t*)dualV_get_owned_ro(view, dual_id_of<skr_render_effect_t>::get());
            if (effects_chunk)
            {
                for (uint32_t i = 0; i < view->count; i++)
                {
                    auto& effects = effects_chunk[i];
                    for (auto& effect : effects)
                    {
                        if (strcmp(effect.name, effect_name) == 0)
                        {
                            batch_render_effects.emplace_back(effect.effect_entity);
                        }
                    }
                }
            }
        };
        dualS_batch(storage, entities, count, DUAL_LAMBDA(game_batch_callback));
        // do cast for render effects
        if (batch_render_effects.size())
        {
            dualS_batch(storage, batch_render_effects.data(), (EIndex)batch_render_effects.size(), view, u);
        }
    }
}
