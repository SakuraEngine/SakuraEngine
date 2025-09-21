#include "SkrProfile/profile.h"
#include "SkrBase/misc/bit.hpp"
#include <SkrContainers/span.hpp>
#include <SkrContainers/string.hpp>
#include <SkrContainers/stl_string.hpp>
#include "SkrRuntime/sugoi/sugoi.h"
#include "SkrRuntime/sugoi/array.hpp"
#include "SkrRuntime/sugoi/set.hpp"
#include "SkrRuntime/sugoi/type_registry.hpp"
#include "SkrRuntime/sugoi/archetype.hpp"
#include "SkrRuntime/sugoi/chunk.hpp"
#include "./impl/query.hpp"

#include "./arena.hpp"
#include "./stack.hpp"
#include "./impl/storage.hpp"

#if __SSE2__
    #include <emmintrin.h>
#endif
#if __SSE4_1__
    #include <smmintrin.h>
#endif
#if __AVX__
    #include <immintrin.h>
#endif
#if __AVX2__
    #include <immintrin.h>
#endif
#if __AVX512F__
    #include <immintrin.h>
#endif
#if defined(__ARM_NEON) || defined(__aarch64__)
    #include <arm_neon.h>
#endif
#include <bit>

namespace skr
{
inline void split(const skr::stl_u8string_view& s, skr::stl_vector<skr::stl_u8string_view>& tokens, const skr::stl_u8string_view& delimiters = u8" ")
{
    skr::stl_string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    skr::stl_string::size_type pos     = s.find_first_of(delimiters, lastPos);
    while (skr::stl_string::npos != pos || skr::stl_string::npos != lastPos)
    {
        auto substr = s.substr(lastPos, pos - lastPos);
        tokens.push_back(substr); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos     = s.find_first_of(delimiters, lastPos);
    }
}

inline bool ends_with(skr::stl_u8string_view const& value, skr::stl_u8string_view const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool starts_with(skr::stl_u8string_view const& value, skr::stl_u8string_view const& starting)
{
    if (starting.size() > value.size()) return false;
    return std::equal(starting.begin(), starting.end(), value.begin());
}
} // namespace skr

namespace sugoi
{
template <class U, class T>
bool match_filter_set(const T& set, const T& all, const T& none, bool skipNone)
{
    if (!set_utils<U>::all(set, all))
        return false;
    if (!skipNone && set_utils<U>::any(set, none))
        return false;
    return true;
}

bool match_group_type(const sugoi_entity_type_t& type, const sugoi_filter_t& filter, bool skipNone)
{
    return match_filter_set<sugoi_type_index_t>(type.type, filter.all, filter.none, skipNone);
}

bool match_group_shared(const sugoi_type_set_t& shared, const sugoi_filter_t& filter)
{
    return match_filter_set<sugoi_type_index_t>(shared, filter.all_shared, filter.none_shared, false);
}

bool match_chunk_changed(const sugoi_chunk_t& chunk, const sugoi_meta_filter_t& filter)
{
    const auto& typeset = chunk.structure->type;

    uint16_t i = 0, j = 0;
    auto&    changed = filter.changed;
    if (changed.length == 0)
        return true;
    while (i < changed.length && j < typeset.length)
    {
        if (changed.data[i] > typeset.data[j])
            j++;
        else if (changed.data[i] < typeset.data[j])
            i++;
        else if (
            const auto timestamp = chunk.get_timestamp_at(j);
            timestamp - filter.timestamp > 0
        )
            return true;
        else
            (j++, i++);
    }
    return false;
}

bool match_group_meta(const sugoi_entity_type_t& type, const sugoi_meta_filter_t& filter)
{
    return match_filter_set<sugoi_entity_t>(type.meta, filter.all_meta, filter.none_meta, false);
}

bool match_group(sugoi_query_t* q, sugoi_group_t* g)
{
    bool match = true;
    match      = match && q->pimpl->includeDead >= g->isDead;
    match      = match && q->pimpl->includeDisabled >= g->disabled;
    if (q->pimpl->overload_cache.excludes.size() > 0)
    {
        for (size_t i = 0; i < q->pimpl->overload_cache.excludes.size(); ++i)
        {
            auto exclude = q->pimpl->overload_cache.excludes[i];
            if (set_utils<sugoi_type_index_t>::all(g->type.type, exclude))
                return false;
        }
    }
    match = match && match_group_type(g->type, q->pimpl->filter, g->archetype->withMask);
    if (!match)
        return false;
    return true;
}
} // namespace sugoi

void sugoi_storage_t::buildQueryCache(sugoi_query_t* q)
{
    using namespace sugoi;
    SkrZoneScopedN("BuildQueryCache");
    q->pimpl->groups_cache.write([&](auto& cache) {
        if (q->pimpl->groups_cache.group_timestamp != pimpl->groups_timestamp)
        {
            cache.clear();
            pimpl->groups.read_versioned([&](auto& groups){
                for (auto i : groups)
                {
                    auto g     = i.second;
                    bool match = sugoi::match_group(q, g);
                    if (!match)
                        continue;
                    cache.push_back(g);
                }
            }, 
            [&](){
                return pimpl->groups_timestamp;
            });
            q->pimpl->groups_cache.group_timestamp = pimpl->groups_timestamp;
        }
    });
}

void sugoi_storage_t::updateQueryCache(sugoi_group_t* group, bool isAdd)
{
    using namespace sugoi;
    SkrZoneScopedN("UpdateQueryCache");
    if (!isAdd)
    {
        pimpl->queries.read_versioned([&](auto& queries){
            for (auto& q : queries)
            {
                q->pimpl->groups_cache.write([&](auto& groups){
                    groups.remove(group);
                });
            }
        }, 
        [&](){
            return pimpl->queries_timestamp;
        });
    }
    else
    {
        pimpl->queries.read_versioned([&](auto& queries){
            for (auto& q : queries)
            {
                if (sugoi::match_group(q, group))
                {
                    q->pimpl->groups_cache.write([&](auto& groups){
                        groups.push_back(group);
                    });
                }
            }
        }, 
        [&](){
            return pimpl->queries_timestamp;
        });
    }
}

//[in][rand]$|comp''
sugoi_query_t* sugoi_storage_t::make_query(const char8_t* inDesc)
{
    using namespace sugoi;
    bool hasAlias = false;
    auto desc = skr::stl_u8string((skr_char8*)inDesc);
    using namespace skr;
#ifdef _WIN32
    desc.erase(std::remove_if(desc.begin(), desc.end(), [](char c) -> bool { return std::isspace(c); }), desc.end());
#else
    desc.erase(std::remove_if(desc.begin(), desc.end(), isspace), desc.end());
#endif
    skr::stl_vector<skr::stl_u8string_view> parts;
    auto                                    spliter = u8",";
    skr::split(desc, parts, spliter);
    // todo: errorMsg? global error code?
    auto&                                              error     = get_error();
    int                                                errorPos  = 0;
    int                                                partBegin = 0;
    auto&                                              reg       = TypeRegistry::get();
    skr::InlineVector<sugoi_type_index_t, 20> all;
    skr::InlineVector<sugoi_type_index_t, 20> none;
    skr::InlineVector<sugoi_type_index_t, 20> all_shared;
    skr::InlineVector<sugoi_type_index_t, 20> none_shared;
    skr::InlineVector<sugoi_type_index_t, 20> entry;
    skr::InlineVector<sugoi_operation_t, 20> operations;
    for (auto part : parts)
    {
        int                i = 0;
        sugoi_type_index_t type;
        sugoi_operation_t  operation;
        bool               shared     = false;
        bool               filterOnly = false;
        operation.randomAccess        = SOS_PAR;
        operation.readonly            = true;
        operation.atomic              = false;
        operation.phase               = -1;
        enum
        {
            OPT,
            ALL,
            NONE
        } selector = ALL;
        if (part[i] == u8'[') // attr: [in] [out] [inout] [has]
        {
            auto j   = i + 1;
            errorPos = partBegin + i;
            while (i < part.size() && part[i] != u8']')
                ++i;
            if (i == part.size())
            {
                error = skr::format(u8"unexpected [ without ], loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            auto attr = part.substr(j, i - j);
            errorPos  = partBegin + j;
            if (attr == u8"in")
                operation.readonly = true;
            else if (attr == u8"inout")
                operation.readonly = false;
            else if (attr == u8"out")
            {
                operation.readonly = false;
                operation.phase    = 0;
            }
            else if (attr == u8"atomic")
            {
                operation.readonly = false;
                operation.atomic   = true;
            }
            else if (attr == u8"has")
                filterOnly = true;
            else
            {
                error = skr::format(u8"unknown access modifier, loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            i++;
        }
        if (i == part.size())
        {
            errorPos = partBegin + i;
            error    = skr::format(u8"unexpected end of part, loc {}.", errorPos);
            SKR_ASSERT(false);
            return nullptr;
        }
        if (part[i] == '<') // attr: <rand> <seq>
        {
            auto j   = i + 1;
            errorPos = partBegin + i;
            while (i < part.size() && part[i] != u8'>')
                ++i;
            if (i == part.size())
            {
                error = skr::format(u8"unexpected [ without ], loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            auto attr = part.substr(j, i - j);
            errorPos  = partBegin + j;
            if (attr == u8"seq")
                operation.randomAccess = SOS_SEQ;
            else if (attr == u8"par")
                operation.randomAccess = SOS_PAR;
            else if (attr == u8"unseq")
            {
                selector               = OPT;
                operation.randomAccess = SOS_UNSEQ;
            }
            else
            {
                error = skr::format(u8"unknown sequence modifier, loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            i++;
        }
        if (i == part.size())
        {
            errorPos = partBegin + i;
            error    = skr::format(u8"unexpected end of part, loc {}.", errorPos);
            SKR_ASSERT(false);
            return nullptr;
        }
        if (!std::isalpha(part[i]))
        {
            if (part[i] == u8'$')
            {
                if (!operation.readonly)
                {
                    errorPos = partBegin + i;
                    error    = skr::format(u8"shared component is readonly, loc {}.", errorPos);
                    SKR_ASSERT(false);
                    return nullptr;
                }
                operation.randomAccess = SOS_SEQ;
                shared                 = true;
                ++i;
            }
            if (operation.randomAccess == SOS_UNSEQ && part[i] != u8'?')
            {
                errorPos = partBegin + i;
                error    = skr::format(u8"unseq component must be optional, loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            if (part[i] == u8'!')
            {
                selector   = NONE;
                filterOnly = true;
            }
            else if (part[i] == u8'?')
                selector = OPT;
            else
            {
                errorPos = partBegin + i;
                error    = skr::format(u8"unknown selector '{}', loc {}.", part[i], errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            ++i;
        }
        if (i == part.size() || !std::isalpha(part[i]))
        {
            errorPos = partBegin + i;
            error    = skr::format(u8"no type specified, loc {}.", errorPos);
            SKR_ASSERT(false);
            return nullptr;
        }
        else
        {
            auto j             = i;
            auto validNameChar = [](char8_t c) {
                return std::isalpha(c) || c == u8'_' || (c > u8'0' && c <= u8'9') || c == u8':';
            };
            while (i < part.size() && validNameChar(part[i]))
                ++i;
            auto name      = part.substr(j, i - j);
            auto name_view = skr::StringView(name.data(), name.size());
            auto iter      = pimpl->overload_data.aliases.find(name_view);
            if (iter != pimpl->overload_data.aliases.end())
            {
                hasAlias = true;
                type = iter->second.type;

                if (operation.phase == 0)
                {
                    errorPos = partBegin + j;
                    error    = skr::format(u8"unexpected phase alias.([out] is always phase 0), loc {}.", errorPos);
                    SKR_ASSERT(false);
                    return nullptr;
                }
                operation.phase = iter->second.phase;
            }
            else
            {
                type = reg.get_type(name_view);
                if (type == kInvalidTypeIndex)
                {
                    errorPos = partBegin + i;
                    error    = skr::format(u8"unknown type name or alias name '{}', loc {}.", name, errorPos);
                    SKR_ASSERT(false);
                    return nullptr;
                }
            }
        }
        if (shared)
        {
            switch (selector)
            {
                case ALL:
                    all_shared.push_back(type);
                    break;
                case NONE:
                    none_shared.push_back(type);
                    break;
                default:
                    break; // optional wont be filtered
            }
        }
        else
        {
            switch (selector)
            {
                case ALL:
                    all.push_back(type);
                    break;
                case NONE:
                    none.push_back(type);
                    break;
                default:
                    break; // optional wont be filtered
            }
        }

        if (!filterOnly)
        {
            entry.push_back(type);
            operations.push_back(operation);
        }

        partBegin += (int)part.size() + 1;
    }

    SKR_DECLARE_ZERO(sugoi_filter_t, filter);
    all.sort(); none.sort();
    all_shared.sort(); none_shared.sort();
    filter.all = { all.data(), (SIndex)all.size() };
    filter.none = { none.data(), (SIndex)none.size() };
    filter.all_shared = { all_shared.data(), (SIndex)all_shared.size() };
    filter.none_shared = { none_shared.data(), (SIndex)none_shared.size() };

    SKR_DECLARE_ZERO(sugoi_parameters_t, params);
    params.types = entry.data();
    params.accesses = operations.data();
    params.length = (SIndex)entry.size();

    auto newQuery = sugoiQ_create(this, &filter, &params);
    const_cast<bool&>(newQuery->pimpl->includeAlias) = hasAlias;
    {
        if (newQuery->pimpl->includeAlias)
            buildQueryOverloads();
    }
    return newQuery;    
}

sugoi_query_t* sugoi_storage_t::make_query(const sugoi_filter_t& filter, const sugoi_parameters_t& params)
{
    using namespace sugoi;

    const auto qsize = sizeof(sugoi_query_t);
    const auto qimplSize = sizeof(sugoi_query_t::Impl);
    const auto bufSize = data_size(filter) + data_size(params);
    auto ptr = sugoi_malloc_aligned(qsize + qimplSize + bufSize, alignof(sugoi_type_index_t));
    auto q = new (ptr) sugoi_query_t();
    auto impl = new ((char*)ptr + qsize) sugoi_query_t::Impl();
    auto buffer = (char*)ptr + qsize + qimplSize;
    q->pimpl = impl;
    q->pimpl->filter = sugoi::clone(filter, buffer);
    q->pimpl->parameters = sugoi::clone(params, buffer);
    q->pimpl->storage = this;
    
    // some pre-compute
    auto at = q->pimpl->filter.all;
    forloop (i, 0, at.length)
    {
        if (at.data[i] == kDeadComponent)
            *const_cast<bool*>(&q->pimpl->includeDead) = true;
        else if (at.data[i] == kDisableComponent)
            *const_cast<bool*>(&q->pimpl->includeDisabled) = true;
    }
    pimpl->queries.update_versioned([&](auto& queries){
        queries.push_back(q);
        pimpl->queries_timestamp += 1;
    }, 
    [&](){
        return pimpl->queries_timestamp;
    });
    buildQueryCache(q);
    return q;
}

void sugoi_storage_t::destroy_query(sugoi_query_t* query)
{
    const bool includeAlias = query->pimpl->includeAlias;
    pimpl->queries.update_versioned(
    [&](auto& queries){
        auto iter = std::find(queries.begin(), queries.end(), query);
        SKR_ASSERT(iter != queries.end());
        query->pimpl->~Impl();
        query->~sugoi_query_t();
        queries.erase(iter);
        sugoi_free_aligned(query, alignof(sugoi_type_index_t));
        pimpl->queries_timestamp += 1;
    }, 
    [&](){
        return pimpl->queries_timestamp;
    });
    if (includeAlias)
        buildQueryOverloads();
}

void sugoi_storage_t::buildQueryOverloads()
{
    using namespace sugoi;
    SkrZoneScopedN("sugoi_storage_t::buildQueryOverloads");
    struct phase_entry_builder {
        sugoi_type_index_t                            type;
        uint32_t                                      phase;
        skr::InlineVector<sugoi_query_t*, 8> queries;
    };
    
    pimpl->overload_data.self_mtx.lock();
    SKR_DEFER({ pimpl->overload_data.self_mtx.unlock(); });
 
    if (pimpl->overload_data.phases != nullptr)
    {
        for (EIndex i = 0; i < pimpl->overload_data.phaseCount; ++i)
            pimpl->overload_data.phases[i]->~phase_entry();
    }
    pimpl->overload_data.phaseCount = 0;
    pimpl->overload_data.queryPhaseArena.reset();
    skr::stl_vector<phase_entry_builder> entries;
    pimpl->queries.read_versioned([&](auto& queries){
        for (auto q : queries)
        {
            auto parameters = q->pimpl->parameters;
            forloop (i, 0, parameters.length)
            {
                if (parameters.accesses[i].phase >= 0 && !parameters.accesses[i].readonly)
                {
                    bool found = false;
                    for (auto& entry : entries)
                    {
                        if (entry.type == parameters.types[i] && entry.phase == parameters.accesses[i].phase)
                        {
                            entry.queries.push_back(q);
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        phase_entry_builder entry;
                        entry.type  = parameters.types[i];
                        entry.phase = parameters.accesses[i].phase;
                        entry.queries.push_back(q);
                        entries.emplace_back(std::move(entry));
                    }
                }
            }
        }
    }, 
    [&](){
        return pimpl->queries_timestamp;
    });

    pimpl->overload_data.phases = pimpl->overload_data.queryPhaseArena.allocate<phase_entry*>(entries.size());
    auto phaseEntries = pimpl->overload_data.phases;
    pimpl->queries.read_versioned([&](auto& queries){
        for (auto q : queries)
        {
            uint32_t count = 0;
            for (auto& entry : entries)
            {
                if (entry.queries.size() < 2)
                    continue;
                if (std::find(entry.queries.begin(), entry.queries.end(), q) != entry.queries.end())
                {
                    count++;
                }
            }
            q->pimpl->overload_cache.phaseCount = 0;
            if (count == 0)
                continue;
            q->pimpl->overload_cache.phases = pimpl->overload_data.queryPhaseArena.allocate<phase_entry*>(count);
        }
    }, 
    [&](){
        return pimpl->queries_timestamp;
    });

    for (auto builder : entries)
    {
        if (builder.queries.size() < 2)
            continue;
        pimpl->overload_data.phaseCount++;
        auto entry        = new (pimpl->overload_data.queryPhaseArena.allocate<phase_entry>(1)) phase_entry();
        (*phaseEntries++) = entry;
        entry->type       = builder.type;
        entry->phase      = builder.phase;
        entry->queries    = { pimpl->overload_data.queryPhaseArena.allocate<sugoi_query_t*>(builder.queries.size()), builder.queries.size() };
        memcpy(entry->queries.data(), builder.queries.data(), sizeof(sugoi_query_t*) * builder.queries.size());

        // solve overloading
        for (int i = 0; i < builder.queries.size(); ++i)
        {
            for (int j = i + 1; j < builder.queries.size(); ++j)
            {
                fixed_stack_scope_t _(localStack);
                auto                a = builder.queries[i];
                auto                b = builder.queries[j];
                // todo: is 256 enough?
                const sugoi_type_index_t* buffer = localStack.allocate<sugoi_type_index_t>(256);
                auto                      merged = set_utils<sugoi_type_index_t>::merge(a->pimpl->filter.all, b->pimpl->filter.all, (void*)buffer);
                SKR_ASSERT(merged.length < 256);
                auto excludeA = set_utils<sugoi_type_index_t>::substract(merged, a->pimpl->filter.all, localStack.allocate<sugoi_type_index_t>(merged.length));
                auto excludeB = set_utils<sugoi_type_index_t>::substract(merged, b->pimpl->filter.all, localStack.allocate<sugoi_type_index_t>(merged.length));
                if (excludeA.length != 0)
                {
                    char* data = (char*)pimpl->overload_data.queryPhaseArena.allocate(data_size(excludeA), alignof(sugoi_type_index_t));
                    a->pimpl->overload_cache.excludes.push_back(sugoi::clone(excludeA, data));
                }
                if (excludeB.length != 0)
                {
                    char* data = (char*)pimpl->overload_data.queryPhaseArena.allocate(data_size(excludeB), alignof(sugoi_type_index_t));
                    b->pimpl->overload_cache.excludes.push_back(sugoi::clone(excludeB, data));
                }
            }
        }
        for (auto query : builder.queries)
        {
            query->pimpl->overload_cache.phases[query->pimpl->overload_cache.phaseCount++] = entry;
        }
    }
}

void sugoi_storage_t::query(const sugoi_query_t* q, sugoi_view_callback_t callback, void* u)
{
    auto filterChunk = [&](sugoi_group_t* group) {
        filter_in_single_group(&q->pimpl->parameters, group, q->pimpl->filter, q->pimpl->meta, q->pimpl->customFilter, q->pimpl->customFilterUserData, callback, u);
    };
    query_groups(q, SUGOI_LAMBDA(filterChunk));
}

void sugoi_storage_t::destroy_entities(const sugoi_query_t* q)
{
    auto filterChunk = [&](sugoi_group_t* group) {
        group->clear();
    };
    query_groups(q, SUGOI_LAMBDA(filterChunk));
}

void sugoi_storage_t::destroy_entities(const sugoi_query_t* q, sugoi_destroy_callback_t callback, void* u)
{
    skr::InlineVector<sugoi_chunk_view_t, 16> viewsToDestroy;
    auto                                      callback3 = [&](sugoi_chunk_view_t* chunk) {
        viewsToDestroy.push_back(*chunk);
    };
    auto filterChunk = [&](sugoi_group_t* group) {
        auto callback2 = [&](sugoi_chunk_view_t* chunk) {
            viewsToDestroy.clear();
            callback(u, chunk, SUGOI_LAMBDA(callback3));
            // perform destroy
            if (viewsToDestroy.size() == 0)
                return;

            // destroy in reverse order
            for (int i = viewsToDestroy.size() - 1; i >= 0; --i)
            {
                destroy_entities(viewsToDestroy[i]);
            }
        };
        filter_in_single_group(&q->pimpl->parameters, group, q->pimpl->filter, q->pimpl->meta, q->pimpl->customFilter, q->pimpl->customFilterUserData, SUGOI_LAMBDA(callback2));
    };
    query_groups(q, SUGOI_LAMBDA(filterChunk));
}

void sugoi_storage_t::query_groups(const sugoi_query_t* q, sugoi_group_callback_t callback, void* u)
{
    using namespace sugoi;
    
    buildQueryCache(const_cast<sugoi_query_t*>(q));
    
    bool                 filterShared  = (q->pimpl->filter.all_shared.length + q->pimpl->filter.none_shared.length) != 0;
    auto& meta = q->pimpl->meta;
    bool filterMeta = (meta.all_meta.length + meta.none_meta.length) != 0;

    sugoi_query_t::Impl::GroupsCache::GroupsCacheVector groups_cache;
    q->pimpl->groups_cache.read([&](auto& val){ groups_cache = val; });
    for (auto& group : groups_cache)
    {
        if (filterShared)
        {
            // check(shared.length < 256);
            if (!match_group_shared(group->sharedType, q->pimpl->filter))
                continue;
        }
        if (filterMeta)
        {
            if (!match_group_meta(group->type, meta))
                continue;
        }
        callback(u, group);
    }
}

bool sugoi_storage_t::match_entity(const sugoi_query_t* q, sugoi_entity_t entity)
{
    using namespace sugoi;
    sugoi_chunk_view_t view = entity_view(entity);
    if (!view.chunk)
        return false;
    auto group = view.chunk->group;
    return match_group(q, group);
}

bool sugoi_storage_t::match_group(const sugoi_query_t* q, const sugoi_group_t* group)
{
    using namespace sugoi;
    bool found = false;
    q->pimpl->groups_cache.read([&](auto& cache){
        auto iter = std::find(cache.begin(), cache.end(), group);
        if (iter != cache.end())
            found = true;
    });
    if (!found)
        return false;
    bool filterShared  = (q->pimpl->filter.all_shared.length + q->pimpl->filter.none_shared.length) != 0;
    auto& meta = q->pimpl->meta;
    bool filterMeta = (meta.all_meta.length + meta.none_meta.length) != 0;
    if (filterShared)
    {
        if (!match_group_shared(group->sharedType, q->pimpl->filter))
            return false;
    }
    if (filterMeta)
    {
        if (!match_group_meta(group->type, meta))
            return false;
    }
    return true;
}

void sugoi_storage_t::filter_groups(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_group_callback_t callback, void* u)
{
    using namespace sugoi;
    bool                 filterShared  = (filter.all_shared.length + filter.none_shared.length) != 0;
    bool filterMeta      = (meta.all_meta.length + meta.none_meta.length) != 0;
    bool includeDead     = false;
    bool includeDisabled = false;
    {
        auto at = filter.all;
        forloop (i, 0, at.length)
        {
            if (at.data[i] == kDeadComponent)
                includeDead = true;
            else if (at.data[i] == kDisableComponent)
                includeDisabled = true;
        }
    }
    auto matchGroup = [&](sugoi_group_t* g) {
        if (includeDead < g->isDead)
            return false;
        if (includeDisabled < g->disabled)
            return false;
        return match_group_type(g->type, filter, g->archetype->withMask);
    };
    
    pimpl->groups.read_versioned([&](auto& groups){
        for (auto& pair : groups)
        {
            auto group = pair.second;
            if (!matchGroup(group))
                continue;
            if (filterShared)
            {
                if (!match_group_shared(group->sharedType, filter))
                    continue;
            }
            if (filterMeta)
            {
                if (!match_group_meta(group->type, meta))
                    continue;
            }
            callback(u, group);
        }
    }, 
    [&](){
        return pimpl->groups_timestamp;
    });
}

bool sugoi_storage_t::match_group(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, const sugoi_group_t* group)
{
    using namespace sugoi;
    bool                 filterShared  = (filter.all_shared.length + filter.none_shared.length) != 0;
    bool filterMeta = (meta.all_meta.length + meta.none_meta.length) != 0;
    if (filterShared)
    {
        if (!match_group_shared(group->sharedType, filter))
            return false;
    }
    if (filterMeta)
    {
        if (!match_group_meta(group->type, meta))
            return false;
    }
    return match_group_type(group->type, filter, group->archetype->withMask);
}

void sugoi_storage_t::filter_in_single_group(const sugoi_parameters_t* params, const sugoi_group_t* group, const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_custom_filter_callback_t customFilter, void* u1, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    bool withCustomFilter = customFilter != nullptr;
    if (!group->archetype->withMask)
    {
        for (auto c : group->chunks)
        {
            if (match_chunk_changed(*c, meta))
            {
                sugoi_chunk_view_t view{ c, (EIndex)0, c->count, params };
                if (!withCustomFilter || customFilter(u1, &view))
                    callback(u, &view);
            }
        }
    }
    else
    {

        auto allmask  = group->get_mask(filter.all);
        auto nonemask = group->get_mask(filter.none);
        // SIMD optimized version for mask matching - choose best available instruction set
#if __AVX512F__
        // AVX-512 optimized version - process 16 masks at once
        const int simd_width = 16;
        // Load allmask and nonemask into AVX-512 registers
        __m512i allmask_vec = _mm512_set1_epi32(allmask);
        __m512i nonemask_vec = _mm512_set1_epi32(nonemask);
        auto match_simd = [&](const sugoi_mask_comp_t* masks) -> __mmask16 {
            
            // Load 16 uint32 masks (512 bits total)
            __m512i mask_vec = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(masks));
            
            // Check (mask & allmask) == allmask
            __m512i and_allmask = _mm512_and_si512(mask_vec, allmask_vec);
            __mmask16 allmask_match = _mm512_cmpeq_epi32_mask(and_allmask, allmask_vec);
            
            // Check (mask & nonemask) == 0
            __m512i and_nonemask = _mm512_and_si512(mask_vec, nonemask_vec);
            __m512i zero_vec = _mm512_setzero_si512();
            __mmask16 nonemask_match = _mm512_cmpeq_epi32_mask(and_nonemask, zero_vec);
            
            // Combine conditions: allmask match AND nonemask match
            return allmask_match & nonemask_match;
        };
        auto check_simd_result = [](auto result) -> bool {
            return result != 0;
        };
        auto check_simd_all_match = [](auto result) -> bool {
            return result == 0xFFFF;
        };
#elif __AVX2__
        // AVX2 optimized version - process 8 masks at once
        const int simd_width = 8;
        // Load allmask and nonemask into AVX2 registers
        __m256i allmask_vec = _mm256_set1_epi32(allmask);
        __m256i nonemask_vec = _mm256_set1_epi32(nonemask);
        auto match_simd = [&](const sugoi_mask_comp_t* masks) -> __m256i {
            
            // Load 8 uint32 masks (256 bits total)
            __m256i mask_vec = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(masks));
            
            // Check (mask & allmask) == allmask
            __m256i and_allmask = _mm256_and_si256(mask_vec, allmask_vec);
            __m256i eq_allmask = _mm256_cmpeq_epi32(and_allmask, allmask_vec);
            
            // Check (mask & nonemask) == 0
            __m256i and_nonemask = _mm256_and_si256(mask_vec, nonemask_vec);
            __m256i zero_vec = _mm256_setzero_si256();
            __m256i eq_zero = _mm256_cmpeq_epi32(and_nonemask, zero_vec);
            
            // Combine conditions: allmask match AND nonemask match
            return _mm256_and_si256(eq_allmask, eq_zero);
        };
        auto check_simd_result = [](auto result) -> bool {
            return _mm256_movemask_epi8(result) != 0;
        };
        auto check_simd_all_match = [](auto result) -> bool {
            return _mm256_movemask_epi8(result) == (int)0xFFFFFFFF;
        };
#elif __AVX__
        // AVX doesn't have integer comparison, fall back to scalar processing
        const int simd_width = 1;
        auto match_simd = [&](const sugoi_mask_comp_t* masks) -> bool {
            return (masks[0] & allmask) == allmask && (masks[0] & nonemask) == 0;
        };
        auto check_simd_result = [](auto result) -> bool {
            return result;
        };
        auto check_simd_all_match = [](auto result) -> bool {
            return result;
        };
#elif defined(__ARM_NEON) || defined(__aarch64__)
        // ARM NEON optimized version - process 4 masks at once
        const int simd_width = 4;
        // Load allmask and nonemask into NEON registers
        uint32x4_t allmask_vec = vdupq_n_u32(allmask);
        uint32x4_t nonemask_vec = vdupq_n_u32(nonemask);
        auto match_simd = [&](const sugoi_mask_comp_t* masks) -> uint32x4_t {
            
            // Load 4 uint32 masks
            uint32x4_t mask_vec = vld1q_u32(masks);
            
            // Check (mask & allmask) == allmask
            uint32x4_t and_allmask = vandq_u32(mask_vec, allmask_vec);
            uint32x4_t eq_allmask = vceqq_u32(and_allmask, allmask_vec);
            
            // Check (mask & nonemask) == 0
            uint32x4_t and_nonemask = vandq_u32(mask_vec, nonemask_vec);
            uint32x4_t zero_vec = vdupq_n_u32(0);
            uint32x4_t eq_zero = vceqq_u32(and_nonemask, zero_vec);
            
            // Combine conditions: allmask match AND nonemask match
            return vandq_u32(eq_allmask, eq_zero);
        };
        auto check_simd_result = [](auto result) -> bool {
            // Check if any lane is true
            uint32x2_t tmp = vorr_u32(vget_low_u32(result), vget_high_u32(result));
            tmp = vpmax_u32(tmp, tmp);
            return vget_lane_u32(tmp, 0) != 0;
        };
        auto check_simd_all_match = [](auto result) -> bool {
            // Check if all lanes are true
            uint32x2_t tmp = vand_u32(vget_low_u32(result), vget_high_u32(result));
            tmp = vpmin_u32(tmp, tmp);
            return vget_lane_u32(tmp, 0) == 0xFFFFFFFF;
        };
#elif __SSE2__
        // SSE2 optimized version - process 4 masks at once
        const int simd_width = 4;
        // Load allmask and nonemask into SSE registers
        __m128i allmask_vec = _mm_set1_epi32(std::bit_cast<int32_t>(allmask));
        __m128i nonemask_vec = _mm_set1_epi32(std::bit_cast<int32_t>(nonemask));
        auto match_simd = [&](const sugoi_mask_comp_t* masks) -> __m128i {
            
            // Load 4 uint32 masks (128 bits total)
            __m128i mask_vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(masks));
            
            // Check (mask & allmask) == allmask
            __m128i and_allmask = _mm_and_si128(mask_vec, allmask_vec);
            __m128i eq_allmask = _mm_cmpeq_epi32(and_allmask, allmask_vec);

            // Check (mask & nonemask) == 0
            __m128i and_nonemask = _mm_and_si128(mask_vec, nonemask_vec);
            __m128i zero_vec = _mm_setzero_si128();
            __m128i eq_zero = _mm_cmpeq_epi32(and_nonemask, zero_vec);
            
            // Combine conditions: allmask match AND nonemask match
            return _mm_and_si128(eq_allmask, eq_zero);
        };
        auto check_simd_result = [](auto result) -> bool {
            return _mm_movemask_epi8(result) != 0;
        };
        auto check_simd_all_match = [](auto result) -> bool {
            return _mm_movemask_epi8(result) == 0xFFFF;
        };
#endif

#if defined(__AVX512F__) || defined(__AVX2__) || defined(__AVX__) || defined(__ARM_NEON) || defined(__aarch64__) || defined(__SSE2__)
        
        auto match_single = [&](sugoi_mask_comp_t mask) {
            return (mask & allmask) == allmask && (mask & nonemask) == 0;
        };
        
        for (auto c : group->chunks)
        {
            if (!match_chunk_changed(*c, meta))
            {
                continue;
            }
            auto count = c->count;
            sugoi_chunk_view_t view = { c, 0, c->count, params };
            auto masks = (sugoi_mask_comp_t*)sugoiV_get_owned_ro(&view, kMaskComponent);
            EIndex i = 0;
            
            while (i < count)
            {
                // Skip non-matching entities using SIMD when possible
                while (i < count)
                {
                    // Use SIMD for bulk processing when we have enough elements
                    if (i + simd_width <= count)
                    {
                        auto result = match_simd(&masks[i]);
                        
                        // If no matches in this SIMD chunk, skip all
                        if (!check_simd_result(result))
                        {
                            i += simd_width;
                            continue;
                        }
                        
                        // Check individual elements in this SIMD chunk
                        bool found_match = false;
                        for (int j = 0; j < simd_width;)
                        {
                            if (match_single(masks[i]))
                            {
                                found_match = true;
                                break;
                            }
                            i++;
                        }
                        if (found_match) break;
                    }
                    else
                    {
                        // Handle remaining elements one by one
                        if (match_single(masks[i]))
                            break;
                        i++;
                    }
                }
                
                view.start = i;
                
                // Find end of matching sequence using SIMD when possible
                while (i < count)
                {
                    if (i + simd_width <= count)
                    {
                        auto result = match_simd(&masks[i]);
                        
                        // If all match in this SIMD chunk, advance by simd_width
                        if (check_simd_all_match(result))
                        {
                            i += simd_width;
                            continue;
                        }
                        
                        // Check individual elements in this SIMD chunk
                        bool found_mismatch = false;
                        for (int j = 0; j < simd_width;)
                        {
                            if (!match_single(masks[i]))
                            {
                                found_mismatch = true;
                                break;
                            }
                            i++;
                        }
                        if (found_mismatch) break;
                    }
                    else
                    {
                        // Handle remaining elements one by one
                        if (!match_single(masks[i]))
                            break;
                        i++;
                    }
                }
                
                view.count = i - view.start;
                if (view.count > 0)
                    if (!withCustomFilter || customFilter(u1, &view))
                        callback(u, &view);
            }
        }
#else
        auto match = [&](sugoi_mask_comp_t mask) {
            return (mask & allmask) == allmask && (mask & nonemask) == 0;
        };
        for (auto c : group->chunks)
        {
            if (!match_chunk_changed(*c, meta))
            {
                continue;
            }
            auto               count = c->count;
            sugoi_chunk_view_t view  = { c, 0, c->count, params };
            auto               masks = (sugoi_mask_comp_t*)sugoiV_get_owned_ro(&view, kMaskComponent);
            EIndex             i     = 0;
            while (i < count)
            {
                while (i < count && !match(masks[i]))
                    ++i;
                view.start = i;
                while (i < count && match(masks[i]))
                    ++i;
                view.count = i - view.start;
                if (view.count > 0)
                    if (!withCustomFilter || customFilter(u1, &view))
                        callback(u, &view);
            }
        }
#endif
    }
}

void sugoi_storage_t::filter_unsafe(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_view_callback_t callback, void* u)
{
    using namespace sugoi;
    auto filterChunk = [&](sugoi_group_t* group) {
        filter_in_single_group(nullptr, group, filter, meta, nullptr, nullptr, callback, u);
    };
    filter_groups(filter, meta, SUGOI_LAMBDA(filterChunk));
}

void sugoi_storage_t::filter_safe(const sugoi_filter_t& filter, const sugoi_meta_filter_t& meta, sugoi_view_callback_t callback, void* u)
{
    SKR_ASSERT(sugoi::ordered(filter));
    SKR_ASSERT(sugoi::ordered(meta));

    if (callback)
        filter_unsafe(filter, meta, callback, u);
}

void sugoi_storage_t::destroy_entities(const sugoi_meta_filter_t& meta)
{
    using namespace sugoi;
    pimpl->groups.read_versioned([&](auto& groups){
        for (auto& [type, group] : groups)
        {
            if (!match_group_meta(group->type, meta))
                continue;
            group->clear();
        }
    }, 
    [&](){
        return pimpl->groups_timestamp;
    });
}

void sugoi_storage_t::destroy_entities(const sugoi_entity_t* ents, EIndex n)
{
    sugoiS_destroy_entities(this, ents, n);
}
