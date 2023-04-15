#include "ecs/type_builder.hpp"
#include "set.hpp"
#include <EASTL/algorithm.h>

namespace dual
{
type_builder_t::type_builder_t()
{
}
type_builder_t::~type_builder_t()
{
}
type_builder_t::type_builder_t(const type_builder_t& other)
{
    indices = other.indices;
}
type_builder_t::type_builder_t(type_builder_t&& other)
{
    indices = std::move(other.indices);
}
type_builder_t& type_builder_t::operator=(const type_builder_t& other)
{
    indices = other.indices;
    return *this;
}
type_builder_t& type_builder_t::operator=(type_builder_t&& other)
{
    indices = std::move(other.indices);
    return *this;
}
type_builder_t& type_builder_t::with(const dual_type_index_t* types, uint32_t inLength)
{
    indices.append(types, types + inLength);
    return *this;
}
void type_builder_t::reserve(uint32_t size)
{
    indices.reserve(size);
}
dual_type_set_t type_builder_t::build()
{
    if(indices.empty())
        return {nullptr, 0};
    std::sort(indices.begin(), indices.end());
    auto end = eastl::unique(indices.begin(), indices.end());
    return { indices.data(), (SIndex)(end - indices.begin()) };
}
} // namespace dual