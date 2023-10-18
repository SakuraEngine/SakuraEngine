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
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}
bool ArrayType::call_dtor(void* ptr) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}
bool ArrayType::call_copy(void* dst, const void* src) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}
bool ArrayType::call_move(void* dst, void* src) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}
bool ArrayType::call_assign(void* dst, const void* src) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}
bool ArrayType::call_move_assign(void* dst, void* src) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}
bool ArrayType::call_hash(const void* ptr, size_t& result) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return true;
}

int ArrayType::write_binary(const void* dst, skr_binary_writer_t* writer) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return 0;
}
int ArrayType::read_binary(void* dst, skr_binary_reader_t* reader) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return 0;
}
void ArrayType::write_json(const void* dst, skr_json_writer_t* writer) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
}
skr::json::error_code ArrayType::read_json(void* dst, skr::json::value_t&& reader) const
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return skr::json::error_code::SUCCESS;
}
} // namespace skr::rttr
