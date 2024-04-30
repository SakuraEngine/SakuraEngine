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

    // TODO. uniform invoke interface
    // TODO. API 查找需要返回一个 Delegate 对象，此功能需要依赖 Delegate

private:
    GUID _generic_guid;
};
} // namespace skr::rttr