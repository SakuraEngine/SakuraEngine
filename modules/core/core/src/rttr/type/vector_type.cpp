#include "SkrCore/log.h"
#include "SkrRTTR/type/vector_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/json/reader.h"

namespace skr::rttr
{
inline static size_t _element_count_of(span<size_t> dimensions)
{
    size_t size = 1;
    for (auto d : dimensions)
    {
        size *= d;
    }
    return size;
}

VectorType::VectorType(Type* target_type, span<size_t> dimensions, skr::String name)
    : GenericType({}, std::move(name), GUID::Create(), target_type->size() * _element_count_of(dimensions), target_type->alignment())
    , _size(_element_count_of(dimensions))
    , _dimensions(dimensions.data(), dimensions.size())
{
}
} // namespace skr::rttr
