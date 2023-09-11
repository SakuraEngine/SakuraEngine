#include "SkrRT/rttr/type_loader/array_type_loader.hpp"
#include "SkrRT/rttr/type/array_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
Type* ArrayTypeLoader::load(Span<TypeDesc> desc)
{
    SKR_ASSERT(desc[0].type() == SKR_TYPE_DESC_TYPE_GUID);
    SKR_ASSERT(desc[0].value_guid() == kArrayGenericGUID);

    // solve dimensions data
    uint64_t dim = desc[1].value_uint64();
    size_t   dimensions_buffer[1000];
    for (int dim_idx = 0; dim_idx < dim; ++dim_idx)
    {
        dimensions_buffer[dim_idx] = desc[dim_idx + 2].value_uint64();
    }

    return SkrNew<ArrayType>(get_type_from_type_desc(desc.subspan(2 + dim)), Span<size_t>{ dimensions_buffer, dim });
}
void ArrayTypeLoader::destroy(Type* type)
{
    SkrDelete(type);
}
} // namespace skr::rttr