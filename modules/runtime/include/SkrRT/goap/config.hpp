#pragma once
#include "SkrBase/template/concepts.hpp"
#include "SkrBase/template/count_member.hpp"
#include "SkrRT/containers/string.hpp"
#include "SkrRT/containers/umap.hpp"

#ifndef SKR_GOAP_SET_NAME
    #ifdef _DEBUG
        #define SKR_GOAP_SET_NAME
    #endif
#endif

namespace skr::goap
{
using CostType     = int64_t;
using PriorityType = float;
using NodeId       = uint64_t;
using NameType     = skr::String;
using OffsetType = decltype(offsetof(skr_guid_t, Storage0));

template <typename Identifier, typename Variable>
using MapType = skr::UMap<Identifier, Variable>;

enum class EConditionType : uint8_t
{
    Equal        = 0x1,
    NotEqual     = 0x2,
    Greater      = 0x3,
    GreaterEqual = 0x4,
    Less         = 0x5,
    LessEqual    = 0x6
};

enum class EVariableFlag : uint8_t
{
    Explicit = 0x1,
    Any      = 0x2,
    None     = 0x3
};

struct SKR_RUNTIME_API Global {
    static NodeId last_id_;
};

} // namespace skr::goap