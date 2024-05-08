#include "SkrRTTR/type/pointer_type.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
PointerType::PointerType(Type* target_type, skr::String name)
    : GenericType({}, std::move(name), GUID::Create(), sizeof(void*), alignof(void*))
{
}
} // namespace skr::rttr