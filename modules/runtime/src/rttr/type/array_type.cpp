#include "SkrRT/rttr/type/array_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
inline static size_t _element_count_of(Span<size_t> dimensions)
{
    size_t size = 1;
    for (auto d : dimensions)
    {
        size *= d;
    }
    return size;
}

ArrayType::ArrayType(Type* target_type, Span<size_t> dimensions, string name)
    : GenericType(kArrayGenericGUID, std::move(name), GUID::Create(), target_type->size() * _element_count_of(dimensions), target_type->alignment())
    , _size(_element_count_of(dimensions))
    , _dimensions(dimensions.data(), dimensions.size())
{
}

bool ArrayType::call_ctor(void* ptr) const
{
    // TODO. impl it
    return true;
}
bool ArrayType::call_dtor(void* ptr) const
{
    // TODO. impl it
    return true;
}
bool ArrayType::call_copy(void* dst, const void* src) const
{
    // TODO. impl it
    return true;
}
bool ArrayType::call_move(void* dst, void* src) const
{
    // TODO. impl it
    return true;
}
bool ArrayType::call_assign(void* dst, const void* src) const
{
    // TODO. impl it
    return true;
}
bool ArrayType::call_move_assign(void* dst, void* src) const
{
    // TODO. impl it
    return true;
}
bool ArrayType::call_hash(const void* ptr, size_t& result) const
{
    // TODO. impl it
    return true;
}

} // namespace skr::rttr
