#pragma once
#include "SkrRT/rttr/type/type.hpp"

namespace skr::rttr
{
struct GenericType : public Type {
    SKR_INLINE GUID generic_guid() { return _generic_guid; }

protected:
    GUID _generic_guid;
};
} // namespace skr::rttr