#pragma once
#include "SkrGuid/guid.hpp"

namespace skr::rttr
{
struct Type;
struct TypeLoader;
struct GenericTypeLoader;

// type register (loader)
SKR_CORE_API void register_type_loader(const GUID& guid, TypeLoader* loader);
SKR_CORE_API void unregister_type_loader(const GUID& guid);

// generic type
// SKR_CORE_API void register_generic_type_loader(const GUID& guid, GenericTypeLoader* type);
// SKR_CORE_API void unregister_generic_type_loader(const GUID& guid);

// get type (after register)
SKR_CORE_API Type* get_type_from_guid(const GUID& guid);

} // namespace skr::rttr