#pragma once
#include "MPShared/world_delta.h"
#include "utils/parallel_for.hpp"
#include "MPShared/components.h"

#include "containers/string.hpp"
#include "containers/vector.hpp"

template<class T, auto F, class H = void, bool bitpacking = false>
skr::task::event_t BuildDelta(dual_type_index_t type, dual_query_t* query, MPWorldDeltaBuildContext ctx, MPWorldDeltaViewBuilder& builder)
{
    static constexpr bool withHistory = !std::is_same_v<H, void>;
    using history_t = std::conditional_t<withHistory, dual::array_comp_T<H, 4>, void>;
    using writer_t = std::conditional_t<bitpacking, skr::binary::VectorWriterBitpacked, skr::binary::VectorWriter>;
    MPComponentDeltaViewBuilder& comps = *std::find_if(builder.components.begin(),builder.components.end(), [&](const MPComponentDeltaViewBuilder& comp)
    {
        return GetNetworkComponent(comp.type) == type;
    });
    skr::task::event_t counter;
    static dual_type_index_t historyComponent = ctx.historyComponent;
    static uint32_t historyComponentSize = withHistory ? dualT_get_desc(historyComponent)->size : 0;skr::task::event_t result{nullptr};
    dual::schedual_custom(query, [ctx, &comps, &builder, type](dual::task_context_t tctx)
    {
        ZoneScopedN("BuildDelta");
        if(comps.entities.empty())
            return;
        
        dual_chunk_view_t lastView {nullptr, 0, 0};
        const T* lastComp = nullptr;
        history_t* lastHistory = nullptr;
        const CAuth* lastAuth = nullptr;
        writer_t writer{&comps.data};
        skr_binary_writer_t archive(writer);

        auto serde = [&](NetEntityId ent) -> bool
        {
            dual_chunk_view_t view = {nullptr, 0, 0};
            dualS_access(tctx.storage, builder.entities[ent], &view);
            const T* comp = nullptr;
            history_t* history = nullptr;
            const CAuth* auth = nullptr;
            if(view.chunk != lastView.chunk)
            {
                lastView = view;
                lastComp = comp = (const T*)tctx.get_owned_ro<T>(&view, 0);
                if constexpr(withHistory)
                {
                    lastHistory = history = (history_t*)tctx.get_owned_rw<H>(&view, 1);
                    lastAuth = auth = (const CAuth*)tctx.get_owned_ro<CAuth>(&view, 2);
                }
            }
            else 
            {
                auto offset = ((int64_t)view.start - lastView.start);
                comp = (const T*)((const char*)lastComp + sizeof(T) * offset);
                if constexpr(withHistory)
                {
                    history = (history_t*)((const char*)lastHistory + historyComponentSize * offset);
                    auth = (const CAuth*)((const char*)lastAuth + sizeof(CAuth) * offset);
                }
            }
            if constexpr(withHistory)
            {
                if(history->size() < ctx.totalConnections + 1)
                    history->resize(ctx.totalConnections + 1);
                return F(view, *comp, (*history)[ctx.connectionId], !auth->initializedConnection[ctx.connectionId], archive);
            }
            else
            {
                F(view, *comp, archive);
            }
            return false;
        };
        if constexpr(withHistory)
            comps.entities.erase(std::remove_if(comps.entities.begin(), comps.entities.end(), serde), comps.entities.end());
        else
        {
            for(auto ent : comps.entities)
                serde(ent);
        }
    }, &counter);
    return counter;
}

template<class T, auto F, bool bitpacking = false>
skr::task::event_t ApplyDelta(dual_type_index_t type, dual_query_t* query, const MPWorldDeltaView& delta, const entity_map_t& map)
{
    auto iter = std::find_if(delta.components.begin(), delta.components.end(), [&](const MPComponentDeltaView & comp) {
            return GetNetworkComponent(comp.type) == type;
        });
    if (iter == delta.components.end())
        return skr::task::event_t(nullptr);
    auto& comps = *iter;
    skr::task::event_t result{nullptr};
    dual::schedual_custom(query, [type, &delta, &map, &comps](dual::task_context_t ctx)
    {
        dual_chunk_view_t lastView {nullptr, 0, 0};
        const T* lastComp = nullptr;
        using reader_t = std::conditional_t<bitpacking, skr::binary::SpanReaderBitpacked, skr::binary::SpanReader>;
        reader_t reader{comps.data};
        skr_binary_reader_t archive(reader);
        for(int i = 0; i < comps.entities.size(); ++i)
        {
            dual_chunk_view_t view;
            auto ei = map.find(delta.entities[comps.entities[i]]);
            SKR_ASSERT(ei != map.end());
            dual_entity_t ent = ei->second;
            dualS_access(ctx.storage, ent, &view);
            T* comp = nullptr;
            if(view.chunk != lastView.chunk)
            {
                lastView = view;
                lastComp = comp = ctx.get_owned_rw<T>(&view, 0);
            }
            else 
            {
                auto offset = sizeof(T) * ((int64_t)view.start - lastView.start);
                comp = (T*)(((const char*)lastComp) + offset);
            }
            F(view, *comp, archive);
        }
    }, &result);
    return result;
}

template<class T>
dual_type_index_t RegisterHistoryComponent()
{
    dual_type_description_t desc;
    auto originDesc = dualT_get_desc(dual_id_of<T>::get());
    skr::string name = skr::format(u8"{}_History", originDesc->name);
    skr::string* persistentName = new skr::string(name);
    desc.name = persistentName->c_str();
    using array_t = dual::array_comp_T<T, 4>;
    desc.size = sizeof(array_t);
    desc.entityFieldsCount = originDesc->entityFieldsCount;
    desc.entityFields = originDesc->entityFields;
    desc.resourceFieldsCount = originDesc->resourceFieldsCount;
    desc.resourceFields = originDesc->resourceFields;
    skr_guid_t guid;
    dual_make_guid(&guid);
    desc.guid = guid;
    desc.flags = 0;
    desc.elementSize = sizeof(T);
    desc.alignment = alignof(array_t);
    desc.callback = originDesc->callback;

    return dualT_register_type(&desc);
}