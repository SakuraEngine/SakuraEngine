#pragma once
#include "SkrBase/types/expected.hpp"
#include "SkrRuntime/ecs/component.hpp"

namespace skr::ecs {

enum class QueryBuilderError
{
    UnknownError,
};

// struct EntityQuery;
using EntityQuery = sugoi_query_t;
using QueryBuilderResult = skr::Expected<QueryBuilderError, EntityQuery*>;

enum class Presence : uint8_t
{
    Own,
    Optional,
    Exclude,
    Share
};

struct QueryBuilder
{
public:
    SKR_RUNTIME_API QueryBuilder(ECSWorld* world) SKR_NOEXCEPT;

    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadWriteAll(Idxs... idxs) { return All<false, Ts...>(idxs...); }
    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadAll(Idxs... idxs) { return All<true, Ts...>(idxs...); }
    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadWriteOptional(Idxs... idxs) { return Optional<false, Ts...>(idxs...); }
    template <typename...Ts, typename...Idxs>
    QueryBuilder& ReadOptional(Idxs... idxs) { return Optional<true, Ts...>(idxs...);  }

    template <skr::ecs::EntityConcept...Ents>
    QueryBuilder& WithMetaEntity(Ents... ents)
    {
        (all_meta.push_back(sugoi_entity_t(ents)), ...);
        return *this;
    }

    template <skr::ecs::EntityConcept...Ents>
    QueryBuilder& WithoutMetaEntity(Ents... ents)
    {
        (none_meta.push_back(sugoi_entity_t(ents)), ...);
        return *this;
    }

    SKR_RUNTIME_API QueryBuilderResult commit() SKR_NOEXCEPT;

    template <typename...Ts, typename...Idxs>
    QueryBuilder& None(Idxs... idxs)
    {
        return AddComponentRequirement<true, Ts..., Idxs...>(Presence::Exclude, idxs...);
    }

    template <bool readonly, typename...Ts, typename...Idxs>
    QueryBuilder& All(Idxs... idxs)
    {
        return AddComponentRequirement<readonly, Ts..., Idxs...>(Presence::Own, idxs...);
    }
    template <bool readonly, typename...Ts, typename...Idxs>
    QueryBuilder& Share(Idxs... idxs)
    {
        return AddComponentRequirement<readonly, Ts..., Idxs...>(Presence::Share, idxs...);
    }
    template <bool readonly, typename...Ts, typename...Idxs>
    QueryBuilder& Optional(Idxs... idxs)
    {
        return AddComponentRequirement<readonly, Ts..., Idxs...>(Presence::Optional, idxs...);
    }

private:
    template <bool readonly, typename...Ts, typename...Idxs>
    QueryBuilder& AddComponentRequirement(Presence presence, Idxs... idxs)
    {
        if constexpr (sizeof...(Ts) > 0)
        {
            if (presence == Presence::Own)
                (all.push_back(idxs), ...);
            else if (presence == Presence::Share)
                (share.push_back(idxs), ...);
            else if (presence == Presence::Exclude)
                (none.push_back(idxs), ...);

            (types.push_back(idxs), ...);
            (ops.push_back({0 * (int)idxs, readonly, false, SOS_SEQ}), ...);
        }
        if (presence == Presence::Own)
            (all.push_back(sugoi_id_of<Ts>::get()), ...);
        else if (presence == Presence::Share)
            (share.push_back(sugoi_id_of<Ts>::get()), ...);
        else if (presence == Presence::Exclude)
            (none.push_back(sugoi_id_of<Ts>::get()), ...);

        (types.push_back(sugoi_id_of<Ts>::get()), ...);
        (ops.push_back({0 * (int)sugoi_id_of<Ts>::get(), readonly, false, SOS_SEQ}), ...);
        return *this;
    }

    ECSWorld* world = nullptr;

    skr::InlineVector<sugoi_entity_t, 4> all_meta;
    skr::InlineVector<sugoi_entity_t, 4> none_meta;

    skr::InlineVector<TypeIndex, 8> all;
    skr::InlineVector<TypeIndex, 4> none;
    skr::InlineVector<TypeIndex, 1> share;
    skr::InlineVector<TypeIndex, 8> types;
    skr::InlineVector<sugoi_operation_t, 8> ops;
};


} // namespace skr::ecs