#include "ecs/type_builder.hpp"
#include "set.hpp"
#include <algorithm>

namespace dual
{
type_builder_t::type_builder_t()
    : data{ nullptr }
    , length{ 0 }
{
}

type_builder_t::~type_builder_t()
{
    if (data != nullptr)
        dual_free(data);
}

type_builder_t& type_builder_t::with(const dual_type_index_t* types, uint32_t inLength)
{
    if (data == nullptr)
        data = (dual_type_index_t*)dual_malloc(sizeof(dual_type_index_t) * inLength);
    else
        data = (dual_type_index_t*)dual_realloc(data, sizeof(dual_type_index_t) * (length + inLength));
    memcpy(data + length, types, inLength * sizeof(dual_type_index_t));
    length += inLength;
    return *this;
}

dual_type_set_t type_builder_t::build()
{
    std::sort(data, data + length);
    return { data, length };
}
} // namespace dual