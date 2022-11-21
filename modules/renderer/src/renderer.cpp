#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "ecs/dual.h"
#include "ecs/array.hpp"
#include "ecs/callback.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderer/render_effect.h"
#include "SkrRenderer/skr_renderer.h"
#include <containers/hashmap.hpp>
#include <EASTL/vector.h>
#include <EASTL/vector_map.h>

struct SKR_RENDERER_API RenderEffectProcessorVtblProxy : public IRenderEffectProcessor {
    RenderEffectProcessorVtblProxy(VtblRenderEffectProcessor vtbl)
        : vtbl(vtbl)
    {
    }

    void on_register(SRendererId renderer, dual_storage_t* storage) override
    {
        if (vtbl.on_register)
            vtbl.on_register(renderer, storage);
    }

    void on_unregister(SRendererId renderer, dual_storage_t* storage) override
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

    void initialize_data(SRendererId renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
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

struct SKR_RENDERER_API SkrRendererImpl : public SRenderer
{
    friend class ::SkrRendererModule;

    SkrRendererImpl(SRenderDeviceId render_device, dual_storage_t* storage) SKR_NOEXCEPT
        : render_device(render_device), storage(storage)
    {

    }

    ~SkrRendererImpl() override
    {
        for (auto proxy : processor_vtbl_proxies)
        {
            if (proxy) SkrDelete(proxy);
        }
    }

    SRenderDeviceId get_render_device() const override
    {
        return render_device;
    }
    
    dual_storage_t* get_dual_storage() const override
    {
        return storage;
    }

    void render(skr::render_graph::RenderGraph* render_graph) override
    {
        // produce draw calls
        for (auto& pass : passes)
        {
            draw_packets[pass->identity()].clear();
            // used out
            for (auto& processor : processors)
            {
                if (pass && processor)
                {
                    auto packet = processor->produce_draw_packets(pass, storage);
                    draw_packets[pass->identity()].emplace_back(packet);
                }
            }
        }
        // execute draw calls
        for (auto& pass : passes)
        {
            if (pass)
            {
                pass->on_register(this, render_graph);

                auto& pass_draw_packets = draw_packets[pass->identity()];
                for (auto pass_draw_packet : pass_draw_packets)
                {
                    for (uint32_t i = 0; i < pass_draw_packet.count; i++)
                    {
                        pass->execute(render_graph, pass_draw_packet.lists[i]);
                    }
                }

                pass->on_unregister(this, render_graph);
            }
        }
    }
    template<typename T>
    using FlatStringMap = skr::flat_hash_map<eastl::string, T, eastl::hash<eastl::string>>;

    eastl::vector<IPrimitiveRenderPass*> passes;
    FlatStringMap<IPrimitiveRenderPass*> passes_map;

    eastl::vector<IRenderEffectProcessor*> processors;
    FlatStringMap<IRenderEffectProcessor*> processors_map;

    eastl::vector<RenderEffectProcessorVtblProxy*> processor_vtbl_proxies;
protected:
    FlatStringMap<eastl::vector<skr_primitive_draw_packet_t>> draw_packets;

    SRenderDevice* render_device = nullptr;
    dual_storage_t* storage = nullptr;
};

SRendererId skr_create_renderer(SRenderDeviceId render_device, dual_storage_t* storage)
{
    return SkrNew<SkrRendererImpl>(render_device, storage);
}

void skr_free_renderer(SRendererId renderer)
{
    SkrDelete(renderer);
}

void skr_renderer_render_frame(SRendererId renderer, skr::render_graph::RenderGraph* render_graph)
{
    renderer->render(render_graph);
}

void skr_renderer_register_render_pass(SRendererId r, skr_render_effect_name_t name, IPrimitiveRenderPass* pass)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& _ = renderer->passes_map.find(name); _ != renderer->passes_map.end())
    {
        SKR_ASSERT(false && "Render pass already registered");
        return;
    }
    renderer->passes_map.emplace(name, pass);
    renderer->passes.emplace_back(pass);
}

void skr_renderer_remove_render_pass(SRendererId r, skr_render_pass_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& pass = renderer->passes_map.find(name); pass != renderer->passes_map.end())
    {
        renderer->passes_map.erase(pass);
        eastl::remove_if(renderer->passes.begin(), renderer->passes.end(), [pass](auto&& p) { return p == pass->second; });
    }
}

void skr_renderer_register_render_effect(SRendererId r, skr_render_effect_name_t name, IRenderEffectProcessor* processor)
{
    auto renderer = (SkrRendererImpl*)r;
    auto storage = renderer->get_dual_storage();
    if (auto&& _ = renderer->processors_map.find(name); _ != renderer->processors_map.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    renderer->processors_map.emplace(name, processor);
    renderer->processors.emplace_back(processor);

    processor->on_register(r, storage);
}

void skr_renderer_register_render_effect_vtbl(SRendererId r, skr_render_effect_name_t name, VtblRenderEffectProcessor* processor)
{
    auto renderer = (SkrRendererImpl*)r;
    auto storage = renderer->get_dual_storage();
    if (auto&& _ = renderer->processors_map.find(name); _ != renderer->processors_map.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    auto proxy = SkrNew<RenderEffectProcessorVtblProxy>(*processor);
    renderer->processors_map.emplace(name, proxy);
    renderer->processor_vtbl_proxies.emplace_back(proxy);
    renderer->processors.emplace_back(static_cast<IRenderEffectProcessor*>(proxy));

    processor->on_register(r, storage);
}

void skr_renderer_remove_render_effect(SRendererId r, skr_render_effect_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    auto storage = renderer->get_dual_storage();
    if (auto&& _ = renderer->processors_map.find(name); _ != renderer->processors_map.end())
    {
        _->second->on_unregister(r, storage);
        renderer->processors_map.erase(_);
        eastl::remove_if(renderer->processors.begin(), renderer->processors.end(), [_](auto&& p) { return p == _->second; });
    }
}

using render_effects_t = dual::array_component_T<skr_render_effect_t, 4>;

void skr_render_effect_attach(SRendererId r, dual_chunk_view_t* g_cv, skr_render_effect_name_t effect_name)
{
    auto renderer = (SkrRendererImpl*)r;
    auto feature_arrs = (render_effects_t*)dualV_get_owned_rw(g_cv, dual_id_of<skr_render_effect_t>::get());
    SKR_ASSERT(feature_arrs && "No render effect component in chunk view");
    if (feature_arrs)
    {
        auto world = renderer->get_dual_storage();
        auto&& i_processor = renderer->processors_map.find(effect_name);

        if (i_processor == renderer->processors_map.end())
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
                    features.emplace_back( skr_render_effect_t{ nullptr, DUAL_NULL_ENTITY } );
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

void skr_render_effect_detach(SRendererId r, dual_chunk_view_t* cv, skr_render_effect_name_t effect_name)
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

void skr_render_effect_add_delta(SRendererId r, const SGameEntity* entities, uint32_t count,
    skr_render_effect_name_t effect_name, dual_delta_type_t delta,
    dual_cast_callback_t callback, void* user_data)
{
    if (count)
    {
        auto storage = r->get_dual_storage();
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

void skr_render_effect_access(SRendererId r, const SGameEntity* entities, uint32_t count, skr_render_effect_name_t effect_name, dual_view_callback_t view, void* u)
{
    if (count)
    {
        auto storage = r->get_dual_storage();
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
