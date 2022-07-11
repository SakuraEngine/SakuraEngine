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

void type_builder_t::with(dual_type_index_t type)
{
    if (data == nullptr)
    {
        data = (dual_type_index_t*)dual_malloc(sizeof(dual_type_index_t));
        data[0] = type;
    }
    else
    {
        data = (dual_type_index_t*)dual_realloc(data, sizeof(dual_type_index_t) * (length + 1));
        data[length++] = type;
    }
}

void type_builder_t::with(dual_type_set_t type)
{
    SIndex i = 0, j = 0, k = 0;
    while (i < length && j < type.length)
    {
        if (data[i] > type.data[j])
            ++j;
        else if (data[i] < type.data[j])
            ++i;
        else
        {
            ++k;
            ++j;
            ++i;
        }
    }
    auto newLength = length + type.length - k;
    data = (dual_type_index_t*)dual_realloc(data, sizeof(dual_type_index_t) * newLength);
    i = 0;
    j = 0;
    k = 0;
    while (i < length && j < type.length)
    {
        if (data[i] > type.data[j])
            data[length + k++] = type.data[j++];
        else if (data[i] < type.data[j])
            ++i;
        else
        {
            ++j;
            ++i;
        }
    }
    while (j < type.length)
        data[length + k++] = type.data[j++];
    std::inplace_merge(data, data + length, data + newLength);
    length = newLength;
}

dual_type_set_t type_builder_t::build()
{
    for (auto i = data; i != data + length; ++i)
        std::rotate(std::upper_bound(data, i, *i), i, std::next(i));
    return { data, length };
}
} // namespace dual