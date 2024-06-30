#pragma once
#include "SkrBase/template/concepts.hpp"
#include "SkrBase/template/count_member.hpp"
#include "SkrBase/template/foreach_field.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/map.hpp"
#include "SkrBase/config.h"

#ifndef SKR_GOAP_SET_NAME
    #ifdef _DEBUG
        #define SKR_GOAP_SET_NAME
    #endif
#endif

#ifndef SKR_GOAP_ERASE_MORE_TYPE
    #define SKR_GOAP_ERASE_MORE_TYPE
#endif

namespace skr::goap
{
using CostType     = int64_t;
using PriorityType = float;
using NodeId       = uint64_t;
using NameType     = skr::String;
using OffsetType   = decltype(offsetof(skr_guid_t, storage0));

template <typename Identifier, typename Variable>
using MapType = skr::Map<Identifier, Variable>;

enum class EConditionType : uint8_t
{
    Equal        = 0x1,
    NotEqual     = 0x2,
    Greater      = 0x3,
    GreaterEqual = 0x4,
    Less         = 0x5,
    LessEqual    = 0x6,
    Exist        = 0x7,
    And          = 0x8,
    Nand         = 0x9
};

enum class EVariableFlag : uint8_t
{
    Explicit = 0x1,
    Optional = 0x2,
    None     = 0x3
};

struct SKR_RUNTIME_API Global {
    static NodeId last_id_;
};

} // namespace skr::goap