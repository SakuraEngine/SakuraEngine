#include "ecs/SmallVector.h"
#include "archetype.hpp"
#include "chunk_view.hpp"

#include "ecs/dual.h"
#include "mask.hpp"
#include "pool.hpp"
#include "query.hpp"
#include "stack.hpp"
#include "storage.hpp"
#include "type.hpp"
#include "type_registry.hpp"
#include "set.hpp"
#include "ecs/constants.hpp"
#include "ecs/callback.hpp"
#include <algorithm>
#include <numeric>
#include "utils/format.hpp"
#include <string>
#include <string_view>
#if __SSE2__
    #include <emmintrin.h>
#endif
#define forloop(i, z, n) for (auto i = std::decay_t<decltype(n)>(z); i < (n); ++i)

namespace eastl
{
inline void split(const string_view& s, vector<string_view>& tokens, const string_view& delimiters = " ")
{
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos)
    {
        auto substr = s.substr(lastPos, pos - lastPos);
        tokens.push_back(substr); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

inline bool ends_with(std::string_view const& value, std::string_view const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

inline bool starts_with(std::string_view const& value, std::string_view const& starting)
{
    if (starting.size() > value.size()) return false;
    return std::equal(starting.begin(), starting.end(), value.begin());
}
} // namespace eastl

namespace dual
{
template <class U, class T>
bool match_filter_set(const T& set, const T& all, const T& any, const T& none, bool skipNone)
{
    if (!set_utils<U>::all(set, all))
        return false;
    if (any.length > 0 && !set_utils<U>::any(set, any))
        return false;
    if (!skipNone && set_utils<U>::any(set, none))
        return false;
    return true;
}

bool match_group_type(const dual_entity_type_t& type, const dual_filter_t& filter, bool skipNone)
{
    return match_filter_set<dual_type_index_t>(type.type, filter.all, filter.any, filter.none, skipNone);
}

bool match_group_shared(const dual_type_set_t& shared, const dual_filter_t& filter)
{
    return match_filter_set<dual_type_index_t>(shared, filter.all_shared, filter.any_shared, filter.none_shared, false);
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
    return match_filter_set<dual_entity_t>(type.meta, filter.all_meta, filter.any_meta, filter.none_meta, false);
}
} // namespace dual

const dual::query_cache_t& dual_storage_t::get_query_cache(const dual_filter_t& filter)
{
    using namespace dual;
    auto iter = queryCaches.find(filter);
    if (iter != queryCaches.end())
        return iter->second;
    query_cache_t cache;
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
    for (auto i : groups)
    {
        auto g = i.second;
        if (matchGroup(g))
            cache.groups.push_back(g);
    }
    cache.includeDead = includeDead;
    cache.includeDisabled = includeDisabled;
    auto totalSize = data_size(filter);
    cache.data.reset(new char[totalSize]);
    char* data = cache.data.get();
    cache.filter = clone(filter, data);
    return queryCaches.emplace(cache.filter, std::move(cache)).first->second;
}

void dual_storage_t::update_query_cache(dual_group_t* group, bool isAdd)
{
    using namespace dual;
    auto match_cache = [&](query_cache_t& cache) {
        if (cache.includeDead < group->isDead)
            return false;
        if (cache.includeDisabled < group->disabled)
            return false;
        return match_group_type(group->type, cache.filter, group->archetype->withMask);
    };
    for (auto& i : queryCaches)
    {
        auto& cache = i.second;
        if (match_cache(cache))
        {
            auto& gs = cache.groups;
            if (!isAdd)
            {
                int j = 0;
                auto count = gs.size();
                for (; j < count && gs[j] != group; ++j)
                    ;
                if (j == count)
                    continue;
                if (j != (count - 1))
                    std::swap(gs[j], gs[count - 1]);
                gs.pop_back();
            }
            else
                gs.push_back({ group });
        }
    }
}

dual_query_t* dual_storage_t::make_query(const dual_filter_t& filter, const dual_parameters_t& params)
{
    using namespace dual;
    auto result = arena.allocate<dual_query_t>();
    auto buffer = (char*)arena.allocate(data_size(filter) + data_size(params), alignof(dual_type_index_t));
    result->filter = clone(filter, buffer);
    result->parameters = clone(params, buffer);
    result->buildedFilter = filter;
    result->built = false;
    result->storage = this;
    queries.push_back(result);
    return result;
}

//[in][rand]$|comp''
dual_query_t* dual_storage_t::make_query(const char* inDesc)
{
    using namespace dual;
    eastl::string desc(inDesc);
#ifdef _WIN32
    desc.erase(std::remove_if(desc.begin(), desc.end(), std::isspace), desc.end());
#else
    desc.erase(std::remove_if(desc.begin(), desc.end(), isspace), desc.end());
#endif
    eastl::vector<eastl::string_view> parts;
    eastl::split(desc, parts, ",");
    // todo: errorMsg? global error code?
    auto& error = get_error();
    int errorPos = 0;
    int partBegin = 0;
    auto& reg = type_registry_t::get();
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> all;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> any;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> none;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> all_shared;
    llvm_vecsmall::SmallVector<dual_type_index_t, 20> any_shared;
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
        operation.randomAccess = DOS_SEQ;
        operation.readonly = true;
        operation.atomic = false;
        operation.phase = -1;
        enum
        {
            OPT,
            ALL,
            ANY,
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
                error = fmt::format("unexpected [ without ], loc {}.", errorPos);
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
                error = fmt::format("unknown access modifier, loc {}.", errorPos);
                return nullptr;
            }
            i++;
        }
        if (part[i] == '[') // attr: [rand] [seq]
        {
            auto j = i + 1;
            errorPos = partBegin + i;
            while (i < part.size() && part[i] != ']')
                ++i;
            if (i == part.size())
            {
                error = fmt::format("unexpected [ without ], loc {}.", errorPos);
                return nullptr;
            }
            auto attr = part.substr(j, j - i);
            errorPos = partBegin + j;
            if (attr.compare("rand") == 0)
                operation.randomAccess = DOS_GLOBAL;
            else if (attr.compare("seq") == 0)
                operation.randomAccess = DOS_SEQ;
            else
            {
                error = fmt::format("unknown sequence modifier, loc {}.", errorPos);
                return nullptr;
            }
            i++;
        }
        if (i == part.size())
        {
            errorPos = partBegin + i;
            error = fmt::format("unexpected end of part, loc {}.", errorPos);
            return nullptr;
        }
        if (!std::isalpha(part[i]))
        {
            if (part[i] == '$')
            {
                if (!operation.readonly)
                {
                    errorPos = partBegin + i;
                    error = fmt::format("shared component is readonly, loc {}.", errorPos);
                    return nullptr;
                }
                operation.randomAccess = DOS_GLOBAL;
                shared = true;
                ++i;
            }
            if (part[i] == '|')
                selector = ANY;
            else if (part[i] == '!')
            {
                selector = NONE;
                filterOnly = true;
            }
            else if (part[i] == '?')
                selector = OPT;
            else
            {
                errorPos = partBegin + i;
                error = fmt::format("unknown selector '{}', loc {}.", part[i], errorPos);
                return nullptr;
            }
            ++i;
        }
        if (i == part.size() || !std::isalpha(part[i]))
        {
            errorPos = partBegin + i;
            error = fmt::format("no type specified, loc {}.", errorPos);
            return nullptr;
        }
        else
        {
            auto j = i;
            while (i < part.size() && std::isalpha(part[i]))
                ++i;
            auto name = part.substr(j, j - i);
            type = reg.get_type(name);
            if (type == kInvalidTypeIndex)
            {
                errorPos = partBegin + i;
                error = fmt::format("unknown type name '{}', loc {}.", name, errorPos);
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
                error = fmt::format("unexpected character, ',' expected, loc {}.", errorPos);
                return nullptr;
            }
            if (operation.phase == 0)
            {
                errorPos = partBegin + j;
                error = fmt::format("unexpected phase modifier.([out] is always phase 0), loc {}.", errorPos);
                return nullptr;
            }
            operation.phase = j - i;
        }
        if (shared)
        {
            switch (selector)
            {
                case ALL:
                    all_shared.push_back(type);
                    break;
                case ANY:
                    any_shared.push_back(type);
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
                case ANY:
                    any.push_back(type);
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

        partBegin += (int)part.size();
    }

    // parse finished, save result into query
    auto result = arena.allocate<dual_query_t>();
#define FILTER_PART(NAME)                \
    std::sort(NAME.begin(), NAME.end()); \
    result->filter.NAME = dual_type_set_t{ NAME.data(), (SIndex)NAME.size() };
    FILTER_PART(all);
    FILTER_PART(any);
    FILTER_PART(none);
    FILTER_PART(all_shared);
    FILTER_PART(any_shared);
    FILTER_PART(none_shared);
#undef FILTER_PART
    auto buffer = (char*)arena.allocate(data_size(result->filter), alignof(dual_type_index_t));
    result->filter = clone(result->filter, buffer);
    result->parameters.types = entry.data();
    result->parameters.accesses = operations.data();
    result->parameters.length = (SIndex)entry.size();
    buffer = (char*)arena.allocate(data_size(result->parameters), alignof(dual_type_index_t));
    result->parameters = clone(result->parameters, buffer);
    result->buildedFilter = result->filter;
    result->storage = this;
    result->built = false;
    std::memset(&result->meta, 0, sizeof(dual_meta_filter_t));
    queries.push_back(result);
    return result;
}

void dual_storage_t::build_queries()
{
    using namespace dual;
    // solve phase collision (overloading)
    struct phase_entry {
        dual_type_index_t type;
        uint32_t phase;
        llvm_vecsmall::SmallVector<dual_query_t*, 8> queries;
    };
    queryBuildArena.reset();
    std::vector<phase_entry> entries;
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
                    phase_entry entry;
                    entry.type = parameters.types[i];
                    entry.phase = parameters.accesses[i].phase;
                    entry.queries.push_back(query);
                    entries.emplace_back(std::move(entry));
                }
            }
        }
        query->buildedFilter = query->filter;
    }
    for (auto entry : entries)
    {
        if (entry.queries.size() < 2)
            continue;
        fixed_stack_scope_t _(localStack);
        dual_type_set_t super;
        super.length = 0;
        // todo: is 256 enough?
        super.data = localStack.allocate<dual_type_index_t>(256);
        const dual_type_index_t* buffer = localStack.allocate<dual_type_index_t>(256);
        // solve overloading
        // algorithm: let super = merge(query.include1...n); queryn.exclude = substract(super, queryn.include)
        // this makes sure only one query can be matched and will cause any typeset cross set boundary became invalid
        // for example, suppose we have query(a, b), query(a, b, c), query(a, b, d)
        // type(a, b, e) will match query(a, b), type(a, b, c) will match query(a, b, c), and type(a, b, c, d) wont match either of them
        for (auto query : entry.queries)
        {
            fixed_stack_scope_t _(localStack);
            auto include = set_utils<dual_type_index_t>::merge(query->buildedFilter.any, query->buildedFilter.all, localStack.allocate<dual_type_index_t>(256));
            auto merged = set_utils<dual_type_index_t>::merge(super, include, (void*)buffer);
            buffer = super.data; // pingpong
            super = merged;
        }
        // check(super.length < 256);
        for (auto query : entry.queries)
        {
            fixed_stack_scope_t _(localStack);
            auto include = set_utils<dual_type_index_t>::merge(query->buildedFilter.any, query->buildedFilter.all, localStack.allocate<dual_type_index_t>(256));
            auto exclude = set_utils<dual_type_index_t>::substract(super, include, localStack.allocate<dual_type_index_t>(super.length));
            auto none = set_utils<dual_type_index_t>::merge(exclude, query->buildedFilter.none, localStack.allocate<dual_type_index_t>(256));
            char* data = (char*)queryBuildArena.allocate(data_size(none), alignof(dual_type_index_t));
            query->buildedFilter.none = clone(none, data);
        }
    }
}

void dual_storage_t::query(const dual_query_t* filter, dual_view_callback_t callback, void* u)
{
    if (!filter->built)
        build_queries();
    return query(filter->buildedFilter, filter->meta, callback, u);
}

void dual_storage_t::query_groups(const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_group_callback_t callback, void* u)
{
    using namespace dual;
    fixed_stack_scope_t _(localStack);
    auto& cache = get_query_cache(filter);
    bool filterShared = (filter.all_shared.length + filter.any_shared.length + filter.none_shared.length) != 0;
    dual_meta_filter_t* validatedMeta = nullptr;
    if (meta.all_meta.length > 0 || meta.any_meta.length > 0 || meta.none_meta.length > 0)
    {
        validatedMeta = localStack.allocate<dual_meta_filter_t>();
        auto data = (char*)localStack.allocate(data_size(meta));
        *validatedMeta = clone(meta, data);
        validate(validatedMeta->all_meta);
        validate(validatedMeta->any_meta);
        validate(validatedMeta->none_meta);
    }
    else
    {
        validatedMeta = (dual_meta_filter_t*)&meta;
    }
    bool filterMeta = (validatedMeta->all_meta.length + validatedMeta->any_meta.length + validatedMeta->none_meta.length) != 0;
    for (auto& group : cache.groups)
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

namespace dual
{
DUAL_FORCEINLINE int CountLeadingZeros64(uint64_t n)
{
#if defined(_MSC_VER) && defined(_M_X64)
    // MSVC does not have __buitin_clzll. Use _BitScanReverse64.
    unsigned long result = 0; // NOLINT(runtime/int)
    if (_BitScanReverse64(&result, n))
    {
        return (int)(63 - result);
    }
    return 64;
#elif defined(_MSC_VER) && !defined(__clang__)
    // MSVC does not have __buitin_clzll. Compose two calls to _BitScanReverse
    unsigned long result = 0; // NOLINT(runtime/int)
    if ((n >> 32) && _BitScanReverse(&result, (unsigned long)(n >> 32)))
    {
        return 31 - result;
    }
    if (_BitScanReverse(&result, (unsigned long)n))
    {
        return 63 - result;
    }
    return 64;
#elif defined(__GNUC__) || defined(__clang__)
    // Use __builtin_clzll, which uses the following instructions:
    //  x86: bsr
    //  ARM64: clz
    //  PPC: cntlzd
    static_assert(sizeof(unsigned long long) == sizeof(n), // NOLINT(runtime/int)
    "__builtin_clzll does not take 64-bit arg");

    // Handle 0 as a special case because __builtin_clzll(0) is undefined.
    if (n == 0)
    {
        return 64;
    }
    return __builtin_clzll(n);
#else
    return CountLeadingZeros64Slow(n);
#endif
}
} // namespace dual

void dual_storage_t::query(const dual_group_t* group, const dual_filter_t& filter, const dual_meta_filter_t& meta, dual_view_callback_t callback, void* u)
{
    using namespace dual;
    if (!group->archetype->withMask)
    {
        dual_chunk_t* c = group->firstChunk;
        while (c != nullptr)
        {
            if (match_chunk_changed(c->type->type, c->timestamps(), meta))
            {
                dual_chunk_view_t view{ c, (EIndex)0, c->count };
                callback(u, &view);
            }
            c = c->next;
        }
    }
    else
    {
        dual_chunk_t* c = group->firstChunk;

        auto allmask = group->get_mask(filter.all);
        auto nonemask = group->get_mask(filter.none);
        auto anymask = group->get_mask(filter.any);
        // todo:benchmark this
#if __SSE2__
        if (nonemask == 0 && anymask == 0) // fastpath
        {
            __m128i allmask_128 = _mm_set1_epi32(allmask);
            while (c != nullptr)
            {
                if (match_chunk_changed(c->type->type, c->timestamps(), meta))
                {
                    c = c->next;
                    continue;
                    ;
                }
                auto count = c->count;
                dual_chunk_view_t view = { c, 0, c->count };
                // todo: ensure mask component aligned
                auto masks = (dual_mask_component_t*)dualV_get_owned_ro(&view, kMaskComponent);
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
                                unsigned long index = dual::CountLeadingZeros64(cmp);
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
                                unsigned long index = dual::CountLeadingZeros64((~cmp) & 0xFFFF);
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
                c = c->next;
            }
        }
        else
#endif
        { // todo: should we simd this snipest too
            auto match = [&](dual_mask_component_t mask) {
                return (mask & allmask) == allmask && (mask & nonemask) == 0 && (anymask == 0 || (mask & anymask) != 0);
            };
            while (c != nullptr)
            {
                if (match_chunk_changed(c->type->type, c->timestamps(), meta))
                {
                    c = c->next;
                    continue;
                    ;
                }
                auto count = c->count;
                dual_chunk_view_t view = { c, 0, c->count };
                auto masks = (dual_mask_component_t*)dualV_get_owned_ro(&view, kMaskComponent);
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
                c = c->next;
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