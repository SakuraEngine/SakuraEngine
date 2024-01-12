#include "SkrRT/misc/log.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/ecs/sugoi.h"
#include "SkrRT/ecs/array.hpp"
#include <SkrRT/containers/hashmap.hpp>
#include "SkrRenderer/render_viewport.h"
#include "SkrRenderer/render_effect.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"

#include "SkrProfile/profile.h"

struct SKR_RENDERER_API RenderEffectProcessorVtblProxy : public IRenderEffectProcessor {
    RenderEffectProcessorVtblProxy(VtblRenderEffectProcessor vtbl)
        : vtbl(vtbl)
    {
    }

    void on_register(SRendererId renderer, sugoi_storage_t* storage) override
    {
        if (vtbl.on_register)
            vtbl.on_register(renderer, storage);
    }

    void on_unregister(SRendererId renderer, sugoi_storage_t* storage) override
    {
        if (vtbl.on_unregister)
            vtbl.on_unregister(renderer, storage);
    }

    void get_type_set(const sugoi_chunk_view_t* cv, sugoi_type_set_t* set) override
    {
        if (vtbl.get_type_set)
            vtbl.get_type_set(cv, set);
    }

    sugoi_type_index_t get_identity_type() override
    {
        if (vtbl.get_identity_type)
            return vtbl.get_identity_type();
        return SUGOI_NULL_TYPE;
    }

    void initialize_data(SRendererId renderer, sugoi_storage_t* storage, sugoi_chunk_view_t* game_cv, sugoi_chunk_view_t* render_cv) override
    {
        if (vtbl.initialize_data)
            vtbl.initialize_data(renderer, storage, game_cv, render_cv);
    }

    skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) override
    {
        skr_primitive_draw_packet_t result = {};
        if (vtbl.produce_draw_packets)
            vtbl.produce_draw_packets(context, &result);
        return result;
    }

    VtblRenderEffectProcessor vtbl;
};

struct SKR_RENDERER_API SkrRendererImpl : public SRenderer
{
    friend class ::SkrRendererModule;

    SkrRendererImpl(SRenderDeviceId render_device, sugoi_storage_t* storage) SKR_NOEXCEPT
        : render_device(render_device), storage(storage)
    {
        viewport_manager = SViewportManager::Create(storage);
    }

    ~SkrRendererImpl() override
    {
        for (auto proxy : processor_vtbl_proxies)
        {
            if (proxy) SkrDelete(proxy);
        }
        SViewportManager::Free(viewport_manager);
    }

    SRenderDeviceId get_render_device() const override
    {
        return render_device;
    }
    
    sugoi_storage_t* get_sugoi_storage() const override
    {
        return storage;
    }

    void render(skr::render_graph::RenderGraph* render_graph) override
    {
        // produce draw calls
        {
            SkrZoneScopedN("ForeachProcessors(Sync)");
            for (auto& processor : processors)
            {
                skr_primitive_update_context_t update_context = {};
                update_context.renderer = this;
                update_context.render_graph = render_graph;
                update_context.storage = storage;

                processor->on_update(&update_context);
            }

            for (auto& pass : passes)
            {
                draw_packets[pass->identity()].clear();
                // used out
                for (auto& processor : processors)
                {
                    if (pass && processor)
                    {
                        SkrZoneScopedN("ProduceDrawPacket");

                        skr_primitive_draw_context_t draw_context = {};
                        draw_context.renderer = this;
                        draw_context.render_graph = render_graph;
                        draw_context.pass = pass;
                        draw_context.storage = storage;

                        auto packet = processor->produce_draw_packets(&draw_context);
                        draw_packets[pass->identity()].add(packet);
                    }
                }
            }

            for (auto& processor : processors)
            {
                skr_primitive_update_context_t update_context = {};
                update_context.renderer = this;
                update_context.render_graph = render_graph;
                update_context.storage = storage;

                processor->post_update(&update_context);
            }
        }

        // execute draw calls
        {
            SkrZoneScopedN("ForeachPasses(Sync)");
            for (auto& pass : passes)
            {
                if (pass)
                {
                    skr_primitive_pass_context_t pass_context = {};
                    pass_context.renderer = this;
                    pass_context.render_graph = render_graph;
                    pass_context.storage = storage;

                    pass->on_update(&pass_context);

                    auto& pass_draw_packets = draw_packets[pass->identity()];
                    {
                        SkrZoneScopedN("PassExecute");

                        pass->execute(&pass_context, {pass_draw_packets.data(), (uint32_t)pass_draw_packets.size()});
                    }

                    pass->post_update(&pass_context);
                }
            }
        }

        passes.clear();
        passes_map.clear();
    }

    SViewportManager* get_viewport_manager() const override
    {
        return viewport_manager;
    }

    SViewportManager* viewport_manager = nullptr;

    template<typename T>
    using FlatStringMap = skr::FlatHashMap<skr::String, T, skr::Hash<skr::String>>;

    skr::Vector<IPrimitiveRenderPass*> passes;
    FlatStringMap<IPrimitiveRenderPass*> passes_map;

    skr::Vector<IRenderEffectProcessor*> processors;
    FlatStringMap<IRenderEffectProcessor*> processors_map;

    skr::Vector<RenderEffectProcessorVtblProxy*> processor_vtbl_proxies;
protected:
    FlatStringMap<skr::Vector<skr_primitive_draw_packet_t>> draw_packets;

    SRenderDevice* render_device = nullptr;
    sugoi_storage_t* storage = nullptr;
};

SRendererId skr_create_renderer(SRenderDeviceId render_device, sugoi_storage_t* storage)
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
    renderer->passes.add(pass);
}

void skr_renderer_remove_render_pass(SRendererId r, skr_render_pass_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    if (auto&& pass = renderer->passes_map.find(name); pass != renderer->passes_map.end())
    {
        renderer->passes_map.erase(pass);
        renderer->passes.remove_all_if(
            [pass](auto&& p) { return p == pass->second; }
        );
    }
}

void skr_renderer_register_render_effect(SRendererId r, skr_render_effect_name_t name, IRenderEffectProcessor* processor)
{
    auto renderer = (SkrRendererImpl*)r;
    auto storage = renderer->get_sugoi_storage();
    if (auto&& _ = renderer->processors_map.find(name); _ != renderer->processors_map.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    renderer->processors_map.emplace(name, processor);
    renderer->processors.add(processor);

    processor->on_register(r, storage);
}

void skr_renderer_register_render_effect_vtbl(SRendererId r, skr_render_effect_name_t name, VtblRenderEffectProcessor* processor)
{
    auto renderer = (SkrRendererImpl*)r;
    auto storage = renderer->get_sugoi_storage();
    if (auto&& _ = renderer->processors_map.find(name); _ != renderer->processors_map.end())
    {
        SKR_ASSERT(false && "Render effect processor already registered");
        return;
    }
    auto proxy = SkrNew<RenderEffectProcessorVtblProxy>(*processor);
    renderer->processors_map.emplace(name, proxy);
    renderer->processor_vtbl_proxies.add(proxy);
    renderer->processors.add(static_cast<IRenderEffectProcessor*>(proxy));

    processor->on_register(r, storage);
}

void skr_renderer_remove_render_effect(SRendererId r, skr_render_effect_name_t name)
{
    auto renderer = (SkrRendererImpl*)r;
    auto storage = renderer->get_sugoi_storage();
    if (auto&& _ = renderer->processors_map.find(name); _ != renderer->processors_map.end())
    {
        _->second->on_unregister(r, storage);
        renderer->processors_map.erase(_);
        renderer->processors.remove_all_if(
            [_](auto&& p) { return p == _->second; }
        );
    }
}

using render_effects_t = sugoi::array_comp_T<skr_render_effect_t, 4>;

void skr_render_effect_attach(SRendererId r, sugoi_chunk_view_t* g_cv, skr_render_effect_name_t effect_name)
{
    auto renderer = (SkrRendererImpl*)r;
    auto feature_arrs = (render_effects_t*)sugoiV_get_owned_rw(g_cv, sugoi_id_of<skr_render_effect_t>::get());
    SKR_ASSERT(feature_arrs && "No render effect component in chunk view");
    if (feature_arrs)
    {
        auto world = sugoiC_get_storage(g_cv->chunk);
        auto&& i_processor = renderer->processors_map.find(effect_name);

        if (i_processor == renderer->processors_map.end())
        {
            SKR_ASSERT(false && "No render effect processor registered");
            return;
        }
        auto entity_type = make_zeroed<sugoi_entity_type_t>();
        i_processor->second->get_type_set(g_cv, &entity_type.type);
        // create render effect entities in storage
        if (entity_type.type.length != 0)
        {
            uint32_t g_id = 0;
            auto initialize_callback = [&](sugoi_chunk_view_t* r_cv) {
                sugoi_chunk_view_t sub_g_cv = *g_cv;
                sub_g_cv.start = g_cv->start + g_id;
                sub_g_cv.count = r_cv->count;
                // do user initialize callback
                i_processor->second->initialize_data(renderer, world, &sub_g_cv, r_cv);
                
                // attach render effect entities to game entities
                auto entities = sugoiV_get_entities(r_cv);
                for (uint32_t i = 0; i < r_cv->count; i++)
                {
                    auto& features = feature_arrs[g_id + i];
    #ifdef _DEBUG
                    for (auto& _ : features)
                    {
                        SKR_ASSERT(strcmp((const char*)_.name, (const char*)effect_name) != 0 && "Render effect already attached");
                    }
    #endif
                    features.emplace_back( skr_render_effect_t{ nullptr, SUGOI_NULL_ENTITY } );
                    auto& feature = features.back();
                    feature.name = effect_name;
                    feature.effect_entity = entities[i];
                }
                g_id += r_cv->count;
            };
            sugoiS_allocate_type(world, &entity_type, g_cv->count, SUGOI_LAMBDA(initialize_callback));
            SKR_ASSERT(g_id == g_cv->count && "Render effect entities count mismatch");
        }
        else
        {
            SKR_LOG_WARN(u8"Render Effect %s privided no valid component types! At least identity type should be provided!", effect_name);
        }
    }
}

void skr_render_effect_detach(SRendererId r, sugoi_chunk_view_t* cv, skr_render_effect_name_t effect_name)
{
    if (cv && cv->count)
    {
        static thread_local skr::Vector<sugoi_entity_t> render_effects;
        render_effects.clear();
        //render_effects.reserve(cv->count);
        auto feature_arrs = (render_effects_t*)sugoiV_get_owned_rw(cv, sugoi_id_of<skr_render_effect_t>::get());
        auto entities = sugoiV_get_entities(cv);
        if (feature_arrs)
        {
            for (uint32_t i = 0; i < cv->count; i++)
            {
                auto& features = feature_arrs[i];
                bool found = false;
                for (auto iter = features.begin(); iter != features.end(); iter++)
                {
                    if (strcmp((const char*)iter->name, (const char*)effect_name) == 0)
                    {
                        render_effects.add(iter->effect_entity);
                        features.erase(iter);
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    SKR_LOG_WARN(u8"Render effect %s not attached to entity %d", effect_name, entities[i]);
                }
            }
        }
        auto storage = sugoiC_get_storage(cv->chunk);
        auto callback = [&](sugoi_chunk_view_t* view) {
            sugoiS_destroy(storage, view);
        };
        sugoiS_batch(r->get_sugoi_storage(), render_effects.data(), (EIndex)render_effects.size(), SUGOI_LAMBDA(callback));
    }
}

void skr_render_effect_add_delta(SRendererId r, sugoi_chunk_view_t* cv,
    skr_render_effect_name_t effect_name, sugoi_delta_type_t delta,
    sugoi_cast_callback_t callback, void* user_data)
{
    if (cv && cv->count)
    {
        auto storage = sugoiC_get_storage(cv->chunk);
        SKR_ASSERT(storage && "No sugoi storage");
        skr::Vector<sugoi_entity_t> render_effects;
        render_effects.reserve(cv->count);
        // batch game ents to collect render effects
        auto feature_arrs = (render_effects_t*)sugoiV_get_owned_rw(cv, sugoi_id_of<skr_render_effect_t>::get());
        if (feature_arrs)
        {
            for (uint32_t i = 0; i < cv->count; i++)
            {
                auto& features = feature_arrs[i];
                for (auto& _ : features)
                {
                    if (strcmp((const char*)_.name, (const char*)effect_name) == 0)
                    {
                        render_effects.add(_.effect_entity);
                    }
                }
            }
        }
        // do cast for render effects
        auto render_batch_callback = [&](sugoi_chunk_view_t* view) {
            sugoiS_cast_view_delta(storage, view, &delta, callback, user_data);
        };
        sugoiS_batch(storage, render_effects.data(), (EIndex)render_effects.size(), SUGOI_LAMBDA(render_batch_callback));
    }
}

void skr_render_effect_access(SRendererId r, sugoi_chunk_view_t* cv, skr_render_effect_name_t effect_name, sugoi_view_callback_t view, void* u)
{
    if (cv && cv->count)
    {
        auto storage = sugoiC_get_storage(cv->chunk);
        SKR_ASSERT(storage && "No sugoi storage");
        skr::Vector<sugoi_entity_t> batch_render_effects;
        batch_render_effects.reserve(cv->count);
        // batch game ents to collect render effects
        auto effects_chunk = (render_effects_t*)sugoiV_get_owned_rw(cv, sugoi_id_of<skr_render_effect_t>::get());
        if (effects_chunk)
        {
            for (uint32_t i = 0; i < cv->count; i++)
            {
                auto& effects = effects_chunk[i];
                for (auto& effect : effects)
                {
                    if (strcmp((const char*)effect.name, (const char*)effect_name) == 0)
                    {
                        batch_render_effects.add(effect.effect_entity);
                    }
                }
            }
        }
        // access render effects
        if (batch_render_effects.size())
        {
            sugoiS_batch(storage, batch_render_effects.data(), (EIndex)batch_render_effects.size(), view, u);
        }
    }
}
