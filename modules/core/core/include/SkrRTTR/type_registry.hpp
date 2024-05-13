#pragma once
#include "SkrGuid/guid.hpp"

namespace skr::rttr
{
struct Type;
using TypeLoaderFunc = void (*)(Type* type);

// type register (loader)
SKR_CORE_API void register_type_loader(const GUID& guid, TypeLoaderFunc load_func);
SKR_CORE_API void unregister_type_loader(const GUID& guid, TypeLoaderFunc load_func);
SKR_CORE_API void unregister_all_type_loader(const GUID& guid);

// TODO. type extender

// TODO. type convert for any support

// generic type
// SKR_CORE_API void register_generic_type_loader(const GUID& guid, GenericTypeLoader* type);
// SKR_CORE_API void unregister_generic_type_loader(const GUID& guid);

// get type (after register)
SKR_CORE_API Type* get_type_from_guid(const GUID& guid);
SKR_CORE_API void  unload_all_types();

} // namespace skr::rttr