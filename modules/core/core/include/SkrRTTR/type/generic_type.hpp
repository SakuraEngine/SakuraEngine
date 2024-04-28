#pragma once
#include "SkrRTTR/type/type.hpp"

namespace skr::rttr
{
struct SKR_CORE_API GenericType : public Type {
    GenericType(GUID generic_id, skr::String name, GUID type_id, size_t size, size_t alignment);

    SKR_INLINE GUID generic_guid()
    {
        return _generic_guid;
    }

private:
    GUID _generic_guid;
};
} // namespace skr::rttr