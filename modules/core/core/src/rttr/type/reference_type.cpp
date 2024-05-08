#include "SkrCore/log.h"
#include "SkrRTTR/type/reference_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
ReferenceType::ReferenceType(Type* target_type, skr::String name)
    : GenericType({}, std::move(name), GUID::Create(), sizeof(size_t&), alignof(size_t&))
{
}

} // namespace skr::rttr