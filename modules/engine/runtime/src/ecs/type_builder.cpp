#include "SkrRT/ecs/type_builder.hpp"
#include <algorithm>

namespace sugoi
{
TypeSetBuilder::TypeSetBuilder()
{
}
TypeSetBuilder::~TypeSetBuilder()
{
}
TypeSetBuilder::TypeSetBuilder(const TypeSetBuilder& other)
{
    indices = other.indices;
}
TypeSetBuilder::TypeSetBuilder(TypeSetBuilder&& other)
{
    indices = std::move(other.indices);
}
TypeSetBuilder& TypeSetBuilder::operator=(const TypeSetBuilder& other)
{
    indices = other.indices;
    return *this;
}
TypeSetBuilder& TypeSetBuilder::operator=(TypeSetBuilder&& other)
{
    indices = std::move(other.indices);
    return *this;
}
TypeSetBuilder& TypeSetBuilder::with(const sugoi_type_index_t* types, uint32_t inLength)
{
    indices.append(types, types + inLength);
    return *this;
}
void TypeSetBuilder::reserve(uint32_t size)
{
    indices.reserve(size);
}
sugoi_type_set_t TypeSetBuilder::build()
{
    if(indices.empty())
        return {nullptr, 0};
    std::sort(indices.begin(), indices.end());
    auto end = std::unique(indices.begin(), indices.end());
    return { indices.data(), (SIndex)(end - indices.begin()) };
}
} // namespace sugoi