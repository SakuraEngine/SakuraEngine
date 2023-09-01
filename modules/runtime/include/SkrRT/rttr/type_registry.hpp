#pragma once
#include "SkrRT/module.configure.h"
#include "SkrBase/containers/misc/span.hpp"
#include "SkrRT/rttr/type_desc.hpp"
#include "SkrRT/rttr/guid.hpp"

namespace skr::rttr
{
struct Type;
struct TypeLoader;

// type register (loader)
// TODO. generic type loader
void register_type_loader(const GUID& guid, TypeLoader* loader);
void unregister_type_loader(const GUID& guid);

// get type (after register)
Type* get_type_from_guid(const GUID& guid);
Type* get_type_from_type_desc(Span<TypeDesc> type_desc);

} // namespace skr::rttr