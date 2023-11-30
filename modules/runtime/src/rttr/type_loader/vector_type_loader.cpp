#include "SkrRT/rttr/type_loader/array_type_loader.hpp"
#include "SkrRT/rttr/type/vector_type.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
Type* VectorTypeLoader::load(Span<TypeDesc> desc)
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

    Type*  target_type = get_type_from_type_desc(desc.subspan(2 + dim));
    string type_name   = target_type->name();
    for (int dim_idx = 0; dim_idx < dim; ++dim_idx)
    {
        type_name.append(format(u8"[{}]", dimensions_buffer[dim_idx]));
    }

    return SkrNew<VectorType>(target_type, Span<size_t>{ dimensions_buffer, dim }, std::move(type_name));
}
void VectorTypeLoader::destroy(Type* type)
{
    SkrDelete(type);
}
} // namespace skr::rttr