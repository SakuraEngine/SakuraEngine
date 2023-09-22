#include "SkrRT/rttr/type/type.hpp"

namespace skr::rttr
{
Type::Type(ETypeCategory type_category, string name, GUID type_id, size_t size, size_t alignment)
    : _type_category(type_category)
    , _name(std::move(name))
    , _type_id(type_id)
    , _size(size)
    , _alignment(alignment)
{
}
} // namespace skr::rttr