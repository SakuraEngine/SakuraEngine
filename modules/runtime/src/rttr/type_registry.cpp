#include "SkrRT/rttr/type_registry.hpp"

namespace skr::rttr
{
// type register (loader)
void register_type_loader(const GUID& guid, TypeLoader* loader)
{
}
void unregister_type_loader(const GUID& guid)
{
}

// TODO. generic type loader

// get type (after register)
Type* get_type_from_guid(const GUID& guid)
{
    return nullptr;
}
Type* get_type_from_type_desc(Span<TypeDesc> type_desc)
{
    return nullptr;
}
} // namespace skr::rttr