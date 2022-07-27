#include "ecs/type_builder.hpp"
#include "set.hpp"
#include <algorithm>

namespace dual
{
type_builder_t& type_builder_t::with(const dual_type_index_t* types, uint32_t inLength)
{
    indices.append(types, types + inLength);
    return *this;
}

dual_type_set_t type_builder_t::build()
{
    std::sort(indices.begin(), indices.end());
    return { indices.data(), (SIndex)indices.size() };
}
} // namespace dual