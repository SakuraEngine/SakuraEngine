#include "type_registry.hpp"
#include "utils/format.hpp"
#include "ecs/SmallVector.h"
#include "archetype.hpp"
#include "arena.hpp"
#include "ecs/dual.h"
#include "query.hpp"
#include "stack.hpp"
#include "storage.hpp"
#include "type.hpp"
#include "set.hpp"
#include "ecs/constants.hpp"

#include <EASTL/string.h>
#include <EASTL/sort.h>
#include <EASTL/algorithm.h>
#include <EASTL/numeric.h>
#include <containers/string.hpp>
#include "utils/bits.hpp"
#include "scheduler.hpp"
#include "containers/span.hpp"
#if __SSE2__
    #include <emmintrin.h>
#endif
#include "internal/utils.hpp"

#include "tracy/Tracy.hpp"

namespace skr
{
inline void split(const eastl::string_view& s, eastl::vector<eastl::string_view>& tokens, const eastl::string_view& delimiters = " ")
{
    eastl::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    eastl::string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (eastl::string::npos != pos || eastl::string::npos != lastPos)
    {
        auto substr = s.substr(lastPos, pos - lastPos);
        tokens.push_back(substr); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

inline bool ends_with(eastl::string_view const& value, eastl::string_view const& ending)
{
    if (ending.size() > value.size()) return false;
    return eastl::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool starts_with(eastl::string_view const& value, eastl::string_view const& starting)
{
    if (starting.size() > value.size()) return false;
    return eastl::equal(starting.begin(), starting.end(), value.begin());
}
} // namespace eastl

namespace dual
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

bool match_group_type(const dual_entity_type_t& type, const dual_filter_t& filter, bool skipNone)
{
    return match_filter_set<dual_type_index_t>(type.type, filter.all, filter.none, skipNone);
}

bool match_group_shared(const dual_type_set_t& shared, const dual_filter_t& filter)
{
    return match_filter_set<dual_type_index_t>(shared, filter.all_shared, filter.none_shared, false);
}

bool match_chunk_changed(const dual_type_set_t& type, uint32_t* timestamp, const dual_meta_filter_t& filter)
{
    uint16_t i = 0, j = 0;
    auto& changed = filter.changed;
    if (changed.length == 0)
        return true;
    while (i < changed.length && j < type.length)
    {
        if (changed.data[i] > type.data[j])
            j++;
        else if (changed.data[i] < type.data[j])
            i++;
        else if (timestamp[j] - filter.timestamp > 0)
            return true;
        else
            (j++, i++);
    }
    return false;
}

bool match_group_meta(const dual_entity_type_t& type, const dual_meta_filter_t& filter)
{
    return match_filter_set<dual_entity_t>(type.meta, filter.all_meta, filter.none_meta, false);
}

bool match_group(dual_query_t* q, dual_group_t* g)
{
    bool match = true;
    match = match && q->includeDead >= g->isDead;
    match = match && q->includeDisabled >= g->disabled;
    if(q->excludes.size() > 0)
    {
        for(int i = 0; i < q->excludes.size(); ++i)
        {
            auto exclude = q->excludes[i];
            if(set_utils<dual_type_index_t>::all(g->type.type, exclude))
                return false;
        }
    }
    match = match && match_group_type(g->type, q->filter, g->archetype->withMask);
    if(!match)
        return false;
    return true;
}
} // namespace dual

void dual_storage_t::build_query_cache(dual_query_t* query)
{
    using namespace dual;
    query->groups.clear();
    query->includeDead = false;
    query->includeDisabled = false;
    {
        auto at = query->filter.all;
        forloop (i, 0, at.length)
        {
            if (at.data[i] == kDeadComponent)
                query->includeDead = true;
            else if (at.data[i] == kDisableComponent)
                query->includeDead = true;
        }
    }
    for (auto i : groups)
    {
        auto g = i.second;
        bool match = dual::match_group(query, g);
        if(!match)
            continue;
        query->groups.push_back(g);
    }
}

void dual_storage_t::update_query_cache(dual_group_t* group, bool isAdd)
{
    using namespace dual;
    
    if (!isAdd)
    {
        for (auto& query : queries)
            query->groups.erase(std::remove(query->groups.begin(), query->groups.end(), group), query->groups.end());
    }
    else 
    {
        for (auto& query : queries)
        {
            if (dual::match_group(query, group))
                query->groups.push_back(group);
        }
    }
}

dual_query_t* dual_storage_t::make_query(const dual_filter_t& filter, const dual_parameters_t& params)
{
    using namespace dual;
    
    dual::fixed_arena_t arena(4096);
    auto result = arena.allocate<dual_query_t>();
    auto buffer = (char*)arena.allocate(data_size(filter) + data_size(params), alignof(dual_type_index_t));
    result->filter = dual::clone(filter, buffer);
    result->parameters = dual::clone(params, buffer);
    queriesBuilt = false;
    result->storage = this;
    arena.forget();
    queries.push_back(result);
    return result;
}

//[in][rand]$|comp''
dual_query_t* dual_storage_t::make_query(const char* inDesc)
{
    using namespace dual;
    eastl::string desc(inDesc);
#ifdef _WIN32
    desc.erase(eastl::remove_if(desc.begin(), desc.end(), [](char c) -> bool { return std::isspace(c); }), desc.end());
#else
    desc.erase(eastl::remove_if(desc.begin(), desc.end(), isspace), desc.end());
#endif
    eastl::vector<eastl::string_view> parts;
    eastl::string spliter = ",";
    skr::split(desc, parts, spliter);
    // todo: errorMsg? global error code?
    auto& error = get_error();
    int errorPos = 0;
    int partBegin = 0;
    auto& reg = type_registry_t::get();
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> all;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> none;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> all_shared;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> none_shared;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> entry;
    llvm_vecsmall::SmallVector<dual_operation_t, 20> operations;
    for (auto part : parts)
    {
        int i = 0;
        dual_type_index_t type;
        dual_operation_t operation;
        bool shared = false;
        bool filterOnly = false;
        operation.randomAccess = DOS_PAR;
        operation.readonly = true;
        operation.atomic = false;
        operation.phase = -1;
        enum
        {
            OPT,
            ALL,
            NONE
        } selector = ALL;
        if (part[i] == '[') // attr: [in] [out] [inout] [has]
        {
            auto j = i + 1;
            errorPos = partBegin + i;
            while (i < part.size() && part[i] != ']')
                ++i;
            if (i == part.size())
            {
                error = skr::format(u8"unexpected [ without ], loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            auto attr = part.substr(j, i - j);
            errorPos = partBegin + j;
            if (attr.compare("in") == 0)
                operation.readonly = true;
            else if (attr.compare("inout") == 0)
                operation.readonly = false;
            else if (attr.compare("out") == 0)
            {
                operation.readonly = false;
                operation.phase = 0;
            }
            else if (attr.compare("atomic") == 0)
            {
                operation.readonly = false;
                operation.atomic = true;
            }
            else if (attr.compare("has") == 0)
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
            error = skr::format(u8"unexpected end of part, loc {}.", errorPos);
            SKR_ASSERT(false);
            return nullptr;
        }
        if (part[i] == '<') // attr: <rand> <seq>
        {
            auto j = i + 1;
            errorPos = partBegin + i;
            while (i < part.size() && part[i] != '>')
                ++i;
            if (i == part.size())
            {
                error = skr::format(u8"unexpected [ without ], loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            auto attr = part.substr(j, i - j);
            errorPos = partBegin + j;
            if (attr.compare("seq") == 0)
                operation.randomAccess = DOS_SEQ;
            else if (attr.compare("par") == 0)
                operation.randomAccess = DOS_PAR;
            else if (attr.compare("unseq") == 0)
            {
                selector = OPT;
                operation.randomAccess = DOS_UNSEQ;
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
            error = skr::format(u8"unexpected end of part, loc {}.", errorPos);
            SKR_ASSERT(false);
            return nullptr;
        }
        if (!std::isalpha(part[i]))
        {
            if (part[i] == '$')
            {
                if (!operation.readonly)
                {
                    errorPos = partBegin + i;
                    error = skr::format(u8"shared component is readonly, loc {}.", errorPos);
                    SKR_ASSERT(false);
                    return nullptr;
                }
                operation.randomAccess = DOS_SEQ;
                shared = true;
                ++i;
            }
            if(operation.randomAccess == DOS_UNSEQ && part[i] != '?')
            {
                errorPos = partBegin + i;
                error = skr::format(u8"unseq component must be optional, loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            if (part[i] == '!')
            {
                selector = NONE;
                filterOnly = true;
            }
            else if (part[i] == '?')
                selector = OPT;
            else
            {
                errorPos = partBegin + i;
                error = skr::format(u8"unknown selector '{}', loc {}.", part[i], errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            ++i;
        }
        if (i == part.size() || !std::isalpha(part[i]))
        {
            errorPos = partBegin + i;
            error = skr::format(u8"no type specified, loc {}.", errorPos);
            SKR_ASSERT(false);
            return nullptr;
        }
        else
        {
            auto j = i;
            auto validNameChar = [](char c)
            {
                return std::isalpha(c) || c =='_' || (c > '0' && c <= '9') || c == ':';
            };
            while (i < part.size() && validNameChar(part[i]))
                ++i;
            auto name = part.substr(j, i - j);
            type = reg.get_type({(const char8_t*)name.data(), name.size()});
            if (type == kInvalidTypeIndex)
            {
                errorPos = partBegin + i;
                error = skr::format(u8"unknown type name '{}', loc {}.", name, errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
        }
        {
            auto j = i;
            while (i < part.size() && part[i] == '\'')
                ++i;
            if (i < part.size())
            {
                errorPos = partBegin + i;
                error = skr::format(u8"unexpected character, ',' expected, loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            if (i > j && operation.phase == 0)
            {
                errorPos = partBegin + j;
                error = skr::format(u8"unexpected phase modifier.([out] is always phase 0), loc {}.", errorPos);
                SKR_ASSERT(false);
                return nullptr;
            }
            if(i > j)
                operation.phase = i - j;
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

    dual::fixed_arena_t arena(4096);
    // parse finished, save result into query
    auto result = arena.allocate<dual_query_t>();
#define FILTER_PART(NAME)                \
    eastl::sort(NAME.begin(), NAME.end()); \
    result->filter.NAME = dual_type_set_t{ NAME.data(), (SIndex)NAME.size() };
    FILTER_PART(all);
    FILTER_PART(none);
    FILTER_PART(all_shared);
    FILTER_PART(none_shared);
#undef FILTER_PART
    auto buffer = (char*)arena.allocate(data_size(result->filter), alignof(dual_type_index_t));
    result->filter = dual::clone(result->filter, buffer);
    result->parameters.types = entry.data();
    result->parameters.accesses = operations.data();
    result->parameters.length = (SIndex)entry.size();
    buffer = (char*)arena.allocate(data_size(result->parameters), alignof(dual_type_index_t));
    result->parameters = dual::clone(result->parameters, buffer);
    result->storage = this;
    queriesBuilt = false;
    ::memset(&result->meta, 0, sizeof(dual_meta_filter_t));
    queries.push_back(result);
    arena.forget();
    return result;
}

void dual_storage_t::destroy_query(dual_query_t* query)
{
    auto iter = eastl::find(queries.begin(), queries.end(), query);
    SKR_ASSERT(iter != queries.end());
    query->~dual_query_t();
    dual_free(query);
    queries.erase(iter);
    queriesBuilt = false;
}

void dual_storage_t::build_queries()
{
    using namespace dual;
    if (queriesBuilt)
        return;
    
    ZoneScopedN("dual_storage_t::build_queries");
    struct phase_entry_builder {
        dual_type_index_t type;
        uint32_t phase;
        llvm_vecsmall::SmallVector<dual_query_t*, 8> queries;
    };
    for(EIndex i = 0; i < phaseCount; ++i)
        phases[i]->~phase_entry();
    phaseCount = 0;
    queryBuildArena.reset();
    eastl::vector<phase_entry_builder> entries;
    for (auto query : queries)
    {
        auto parameters = query->parameters;
        forloop (i, 0, parameters.length)
        {
            if (parameters.accesses[i].phase >= 0 && !parameters.accesses[i].readonly)
            {
                bool found = false;
                for (auto& entry : entries)
                {
                    if (entry.type == parameters.types[i] && entry.phase == parameters.accesses[i].phase)
                    {
                        entry.queries.push_back(query);
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    phase_entry_builder entry;
                    entry.type = parameters.types[i];
                    entry.phase = parameters.accesses[i].phase;
                    entry.queries.push_back(query);
                    entries.emplace_back(std::move(entry));
                }
            }
        }
    }
    phases = queryBuildArena.allocate<phase_entry*>(entries.size());
    auto phaseEntries = phases;
    for (auto query : queries)
    {
        uint32_t count = 0;
        for (auto& entry : entries)
        {
            if (entry.queries.size() < 2)
                continue;
            if(std::find(entry.queries.begin(), entry.queries.end(), query) != entry.queries.end())
            {
                count++;
            }
        }
        query->phaseCount = 0;
        if(count == 0)
            continue;
        query->phases = queryBuildArena.allocate<phase_entry*>(count);
    }
    for (auto builder : entries)
    {
        if (builder.queries.size() < 2)
            continue;
        phaseCount ++;
        auto entry = new (queryBuildArena.allocate<phase_entry>(1)) phase_entry(); 
        (*phaseEntries++) = entry;
        entry->type = builder.type;
        entry->phase = builder.phase;
        entry->queries = {queryBuildArena.allocate<dual_query_t*>(builder.queries.size()), builder.queries.size()};
        memcpy(entry->queries.data(), builder.queries.data(), sizeof(dual_query_t*) * builder.queries.size());

        // solve overloading
        for(int i = 0; i < builder.queries.size(); ++i)
        {
            for(int j = i+1; j < builder.queries.size(); ++j)
            {
                fixed_stack_scope_t _(localStack);
                auto a = builder.queries[i];
                auto b = builder.queries[j];
                // todo: is 256 enough?
                const dual_type_index_t* buffer = localStack.allocate<dual_type_index_t>(256);
                auto merged = set_utils<dual_type_index_t>::merge(a->filter.all, b->filter.all, (void*)buffer);
                SKR_ASSERT(merged.length < 256);
                auto excludeA = set_utils<dual_type_index_t>::substract(merged, a->filter.all, localStack.allocate<dual_type_index_t>(merged.length));
                auto excludeB = set_utils<dual_type_index_t>::substract(merged, b->filter.all, localStack.allocate<dual_type_index_t>(merged.length));
                if(excludeA.length == 0)
                {
                    char* data = (char*)queryBuildArena.allocate(data_size(excludeA), alignof(dual_type_index_t));
                    a->excludes.push_back(dual::clone(excludeA, data));
                }
                if(excludeB.length == 0)
                {
                    char* data = (char*)queryBuildArena.allocate(data_size(excludeB), alignof(dual_type_index_t));
                    b->excludes.push_back(dual::clone(excludeB, data));
                }
            }
        }
        for(auto query : builder.queries)
        {
            query->phases[query->phaseCount++] = entry;
        }
    }
    // build query cache
    for(auto& query : queries)
    {
        build_query_cache(query);
    }
    
    queriesBuilt = true;
}

void dual_storage_t::query(const dual_query_t* q, dual_view_callback_t callback, void* u)
{
    bool mainThread = true;
    if(scheduler)
    {
        mainThread = scheduler->is_main_thread(this);
    }
    if(mainThread)
    {
        build_queries();
    }
    else
        SKR_ASSERT(queriesBuilt);
    
    auto filterChunk = [&](dual_group_t* group) {
        query(group, q->filter, q->meta, callback, u);
    };
    query_groups(q, DUAL_LAMBDA(filterChunk));
}


void dual_storage_t::query_groups(const dual_query_t* q, dual_group_callback_t callback, void* u)
{
    using namespace dual;
    bool mainThread = true;
    if(scheduler)
    {
        mainThread = scheduler->is_main_thread(this);
    }
    if(mainThread)
    {
        build_queries();
    }
    else
        SKR_ASSERT(queriesBuilt);
    
    fixed_stack_scope_t _(localStack);
    bool filterShared = (q->filter.all_shared.length + q->filter.none_shared.length) != 0;
    dual_meta_filter_t* validatedMeta = nullptr;
    if (q->meta.all_meta.length > 0 || q->meta.any_meta.length > 0 || q->meta.none_meta.length > 0)
    {
        validatedMeta = localStack.allocate<dual_meta_filter_t>();
        auto data = (char*)localStack.allocate(data_size(q->meta));
        *validatedMeta = dual::clone(q->meta, data);
        validate(validatedMeta->all_meta);
        validate(validatedMeta->any_meta);
        validate(validatedMeta->none_meta);
    }
    else
    {
        validatedMeta = (dual_meta_filter_t*)&q->meta;
    }
    bool filterMeta = (validatedMeta->all_meta.length + validatedMeta->any_meta.length + validatedMeta->none_meta.length) != 0;
    for (auto& group : q->groups)
    {
        if (filterShared)
        {
            fixed_stack_scope_t _(localStack);
            dual_type_set_t shared;
            shared.length = 0;
            // todo: is 256 enough?
            shared.data = localStack.allocate<dual_type_index_t>(256);
            group->get_shared_type(shared, localStack.allocate<dual_type_index_t>(256));
            // check(shared.length < 256);
            if (!match_group_shared(shared, q->filter))
                continue;
        }
        if (filterMeta)
        {
            if (!match_group_meta(group->type, *validatedMeta))
                continue;
        }
        callback(u, group);
    }
}

void dual_storage_t::query_groups(const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_group_callback_t callback, void* u)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    bool filterShared = (filter.all_shared.length + filter.none_shared.length) != 0;
    dual_meta_filter_t* validatedMeta = nullptr;
    if (meta.all_meta.length > 0 || meta.any_meta.length > 0 || meta.none_meta.length > 0)
    {
        validatedMeta = localStack.allocate<dual_meta_filter_t>();
        auto data = (char*)localStack.allocate(data_size(meta));
        *validatedMeta = dual::clone(meta, data);
        validate(validatedMeta->all_meta);
        validate(validatedMeta->any_meta);
        validate(validatedMeta->none_meta);
    }
    else
    {
        validatedMeta = (dual_meta_filter_t*)&meta;
    }
    bool filterMeta = (validatedMeta->all_meta.length + validatedMeta->any_meta.length + validatedMeta->none_meta.length) != 0;
    bool includeDead = false;
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
    auto matchGroup = [&](dual_group_t* g) {
        if (includeDead < g->isDead)
            return false;
        if (includeDisabled < g->disabled)
            return false;
        return match_group_type(g->type, filter, g->archetype->withMask);
    };
    for (auto& pair : groups)
    {
        auto group = pair.second;
        if(!matchGroup(group))
            continue;
        if (filterShared)
        {
            fixed_stack_scope_t _(localStack);
            dual_type_set_t shared;
            shared.length = 0;
            // todo: is 256 enough?
            shared.data = localStack.allocate<dual_type_index_t>(256);
            group->get_shared_type(shared, localStack.allocate<dual_type_index_t>(256));
            // check(shared.length < 256);
            if (!match_group_shared(shared, filter))
                continue;
        }
        if (filterMeta)
        {
            if (!match_group_meta(group->type, *validatedMeta))
                continue;
        }
        callback(u, group);
    }
}

bool dual_storage_t::match_group(const dual_filter_t& filter, const dual_meta_filter_t& meta, const dual_group_t* group)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    bool filterShared = (filter.all_shared.length + filter.none_shared.length) != 0;
    dual_meta_filter_t* validatedMeta = nullptr;
    if (meta.all_meta.length > 0 || meta.any_meta.length > 0 || meta.none_meta.length > 0)
    {
        validatedMeta = localStack.allocate<dual_meta_filter_t>();
        auto data = (char*)localStack.allocate(data_size(meta));
        *validatedMeta = dual::clone(meta, data);
        validate(validatedMeta->all_meta);
        validate(validatedMeta->any_meta);
        validate(validatedMeta->none_meta);
    }
    else
    {
        validatedMeta = (dual_meta_filter_t*)&meta;
    }
    bool filterMeta = (validatedMeta->all_meta.length + validatedMeta->any_meta.length + validatedMeta->none_meta.length) != 0;
    if (filterShared)
    {
        fixed_stack_scope_t _(localStack);
        dual_type_set_t shared;
        shared.length = 0;
        // todo: is 256 enough?
        shared.data = localStack.allocate<dual_type_index_t>(256);
        group->get_shared_type(shared, localStack.allocate<dual_type_index_t>(256));
        // check(shared.length < 256);
        if (!match_group_shared(shared, filter))
            return false;
    }
    if (filterMeta)
    {
        if (!match_group_meta(group->type, *validatedMeta))
            return false;
    }
    return match_group_type(group->type, filter, group->archetype->withMask);
}

void dual_storage_t::query(const dual_group_t* group, const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    if (!group->archetype->withMask)
    {
        for(auto c : group->chunks)
        {
            if (match_chunk_changed(c->type->type, c->timestamps(), meta))
            {
                dual_chunk_view_t view{ c, (EIndex)0, c->count };
                callback(u, &view);
            }
        }
    }
    else
    {

        auto allmask = group->get_mask(filter.all);
        auto nonemask = group->get_mask(filter.none);
        // todo:benchmark this
#if __SSE2__
        if (nonemask == 0) // fastpath
        {
            __m128i allmask_128 = _mm_set1_epi32(allmask);
            for(auto c : group->chunks)
            {
                if (!match_chunk_changed(c->type->type, c->timestamps(), meta))
                {
                    continue;
                }
                auto count = c->count;
                dual_chunk_view_t view = { c, 0, c->count };
                auto masks = (dual_mask_comp_t*)dualV_get_owned_ro(&view, kMaskComponent);
                EIndex i = 0;
                while (i < count)
                {
                    while (i < count && !((masks[i] & allmask) == allmask) && i % 4 != 0)
                        ++i;
                    if (i % 4 == 0)
                    {
                        while (i < count - 4)
                        {
                            __m128i m = _mm_load_si128((__m128i*)(masks + i));
                            m = _mm_and_si128(allmask_128, m);
                            uint16_t cmp = _mm_movemask_epi8(_mm_cmpeq_epi32(m, allmask_128));
                            if (cmp == 0)
                            {
                                i += 4;
                                continue;
                            }
                            else
                            {
                                unsigned long index = skr::CountLeadingZeros64(cmp);
                                i += index / 4;
                                break;
                            }
                        }
                        if (i >= count - 4)
                            while (i < count && !((masks[i] & allmask) == allmask))
                                ++i;
                    }
                    view.start = i;

                    while (i < count && (masks[i] & allmask) == allmask && i % 4 != 0)
                        ++i;
                    if (i % 4 == 0)
                    {
                        while (i < count - 4)
                        {
                            __m128i m = _mm_load_si128((__m128i*)(masks + i));
                            m = _mm_and_si128(allmask_128, m);
                            uint16_t cmp = _mm_movemask_epi8(_mm_cmpeq_epi32(m, allmask_128));
                            if (cmp == 0xFFFF)
                            {
                                i += 4;
                                continue;
                            }
                            else
                            {
                                unsigned long index = skr::CountLeadingZeros64((~cmp) & 0xFFFF);
                                i += index / 4;
                                break;
                            }
                        }
                        if (i >= count - 4)
                            while (i < count && ((masks[i] & allmask) == allmask))
                                ++i;
                    }
                    view.count = i - view.start;
                    if (view.count > 0)
                        callback(u, &view);
                }
            }
        }
        else
#endif
        { // todo: should we simd this snipest too
            auto match = [&](dual_mask_comp_t mask) {
                return (mask & allmask) == allmask && (mask & nonemask) == 0;
            };
            for(auto c : group->chunks)
            {
                if (!match_chunk_changed(c->type->type, c->timestamps(), meta))
                {
                    continue;
                }
                auto count = c->count;
                dual_chunk_view_t view = { c, 0, c->count };
                auto masks = (dual_mask_comp_t*)dualV_get_owned_ro(&view, kMaskComponent);
                EIndex i = 0;
                while (i < count)
                {
                    while (i < count && !match(masks[i]))
                        ++i;
                    view.start = i;
                    while (i < count && match(masks[i]))
                        ++i;
                    view.count = i - view.start;
                    if (view.count > 0)
                        callback(u, &view);
                }
            }
        }
    }
}

void dual_storage_t::query(const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    auto filterChunk = [&](dual_group_t* group) {
        query(group, filter, meta, callback, u);
    };
    query_groups(filter, meta, DUAL_LAMBDA(filterChunk));
}

void dual_storage_t::destroy(const dual_meta_filter_t& meta)
{
    using namespace dual;
    for (auto& pair : groups)
    {
        auto group = pair.second;
        if (!match_group_meta(group->type, meta))
            continue;
        group->clear();
    }
}