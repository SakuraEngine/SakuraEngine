#include "SkrRT/rttr/type/generic_type.hpp"

namespace skr::rttr
{
GenericType::GenericType(GUID generic_id, string name, GUID type_id, size_t size, size_t alignment)
    : Type(ETypeCategory::SKR_TYPE_CATEGORY_GENERIC, std::move(name), type_id, size, alignment)
    , _generic_guid(generic_id)
{
}
} // namespace skr::rttr