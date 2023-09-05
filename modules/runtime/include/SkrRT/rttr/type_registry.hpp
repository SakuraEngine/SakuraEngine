#pragma once
#include "SkrRT/module.configure.h"
#include "SkrRT/containers_new/span.hpp"
#include "SkrRT/rttr/type_desc.hpp"
#include "SkrRT/rttr/guid.hpp"

namespace skr::rttr
{
struct Type;
struct TypeLoader;
struct GenericTypeLoader;

// type register (loader)
RUNTIME_API void register_type_loader(const GUID& guid, TypeLoader* loader);
RUNTIME_API void unregister_type_loader(const GUID& guid);

// generic type loader
RUNTIME_API void register_generic_type_loader(const GUID& generic_guid, GenericTypeLoader* loader);
RUNTIME_API void unregister_generic_type_loader(const GUID& generic_guid);

// get type (after register)
RUNTIME_API Type* get_type_from_guid(const GUID& guid);
RUNTIME_API Type* get_type_from_type_desc(Span<TypeDesc> type_desc);

} // namespace skr::rttr