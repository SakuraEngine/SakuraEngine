#include "MPShared/world_delta.h"
#include "containers/vector.hpp"
#include "utils/parallel_for.hpp"
#include "ecs/callback.hpp"
#include "components.h"
#include "containers/string.hpp"
#include "utils/format.hpp"



template<class T, auto F, bool withHistory=false>
skr::task::event_t BuildDelta(dual_type_index_t type, dual_query_t* query, MPWorldDeltaBuildContext ctx, MPWorldDeltaViewBuilder& builder)
{
    MPComponentDeltaViewBuilder& comps = *std::find_if(builder.components.begin(),builder.components.end(), [&](const MPComponentDeltaViewBuilder& comp)
    {
        return GetNetworkComponent(comp.type) == type;
    });
    skr::vector<skr::task::event_t> dependencies;
    skr::task::event_t counter;
    auto callback = [&](dual_counter_t* counter)
    {
        dependencies.push_back(*(skr::task::event_t*)counter);
        dualJ_release_counter(counter);
    };
    static dual_type_index_t historyComponent = ctx.historyComponent;
    dualJ_schedule_custom(query, (dual_counter_t*)&counter, DUAL_LAMBDA(callback), nullptr);
    auto storage = dualQ_get_storage(query);
    skr::task::schedule([dependencies = std::move(dependencies), storage, ctx, &comps, &builder, type]
    ()
    {
        ZoneScopedN("BuildDelta");
        for(auto& dep : dependencies)
            dep.wait(false);
        if(comps.entities.empty())
            return;
        
        dual_chunk_view_t lastView {nullptr, 0, 0};
        const T* lastComp = nullptr;
        dual::array_comp_T<T, 4>* lastHistory = nullptr;
        const CAuth* lastAuth = nullptr;
        skr::binary::VectorWriterBitpacked writer{&comps.data};
        skr_binary_writer_t archive(writer);
        comps.entities.erase(std::remove_if(comps.entities.begin(), comps.entities.end(), [&](NetEntityId ent) -> bool
        {
            dual_chunk_view_t view = {nullptr, 0, 0};
            dualS_access(storage, builder.entities[ent], &view);
            const T* comp = nullptr;
            dual::array_comp_T<T, 4>* history = nullptr;
            const CAuth* auth = nullptr;
            if(view.chunk != lastView.chunk)
            {
                lastView = view;
                lastComp = comp = (const T*)dualV_get_owned_ro(&view, type);
                if constexpr(withHistory)
                {
                    lastHistory = history = (dual::array_comp_T<T, 4>*)dualV_get_owned_ro(&view, historyComponent);
                    lastAuth = auth = (const CAuth*)dualV_get_owned_ro(&view, dual_id_of<CAuth>::get());
                }
            }
            else 
            {
                auto offset = ((int64_t)view.start - lastView.start);
                comp = (const T*)((const char*)lastComp + sizeof(T) * offset);
                if constexpr(withHistory)
                {
                    history = (dual::array_comp_T<T, 4>*)((const char*)lastHistory + sizeof(dual::array_comp_T<T, 4>) * offset);
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
        }), comps.entities.end());
    }, &counter);
    return counter;
}

template<class T, auto F>
skr::task::event_t ApplyDelta(dual_type_index_t type, dual_query_t* query, const MPWorldDeltaView& delta, const entity_map_t& map)
{
    auto iter = std::find_if(delta.components.begin(), delta.components.end(), [&](const MPComponentDeltaView & comp) {
            return GetNetworkComponent(comp.type) == type;
        });
    if (iter == delta.components.end())
        return skr::task::event_t(nullptr);
    auto& comps = *iter;
    skr::vector<skr::task::event_t> dependencies;
    skr::task::event_t counter;
    auto callback = [&](dual_counter_t* counter) {
        dependencies.push_back(*(skr::task::event_t*)counter);
        dualJ_release_counter(counter);
    };
    dualJ_schedule_custom(query, (dual_counter_t*)&counter, DUAL_LAMBDA(callback), nullptr);
    auto storage = dualQ_get_storage(query);
    skr::task::schedule(
    [dependencies = std::move(dependencies), type, &delta, &map, &comps, storage]() {
        ZoneScopedN("ApplyDelta");
        for (auto& dep : dependencies)
            dep.wait(false);
        dual_chunk_view_t lastView {nullptr, 0, 0};
        const T* lastComp = nullptr;
        skr::binary::SpanReaderBitpacked reader{comps.data};
        skr_binary_reader_t archive(reader);
        for(int i = 0; i < comps.entities.size(); ++i)
        {
            dual_chunk_view_t view;
            auto ei = map.find(delta.entities[comps.entities[i]]);
            SKR_ASSERT(ei != map.end());
            dual_entity_t ent = ei->second;
            dualS_access(storage, ent, &view);
            T* comp = nullptr;
            if(view.chunk != lastView.chunk)
            {
                lastView = view;
                lastComp = comp = (T*)dualV_get_owned_rw(&view, type);
            }
            else 
            {
                auto offset = sizeof(T) * ((int64_t)view.start - lastView.start);
                comp = (T*)(((const char*)lastComp) + offset);
            }
            F(view, *comp, archive);
        }
    },
    &counter);
    return counter;
}

template<class T>
dual_type_index_t RegisterHistoryComponent()
{
    dual_type_description_t desc;
    auto originDesc = dualT_get_desc(dual_id_of<T>::get());
    skr::string name = skr::format("{}_History", originDesc->name);
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