#include "SkrRT/rttr/type_registry.hpp"
#include "SkrRT/containers_new/umap.hpp"
#include "SkrRT/misc/log.h"
#include "SkrRT/rttr/type_loader/type_loader.hpp"
#include "SkrRT/rttr/type_loader/generic_type_loader.hpp"
#include "SkrRT/rttr/type/type.hpp"

namespace skr::rttr
{
static UMap<GUID, TypeLoader*>& type_loaders()
{
    static UMap<GUID, TypeLoader*> s_type_loaders;
    return s_type_loaders;
}
static UMap<GUID, Type*>& loaded_types()
{
    static UMap<GUID, Type*> s_types;
    return s_types;
}

// type register (loader)
void register_type_loader(const GUID& guid, TypeLoader* loader)
{
    auto result = type_loaders().add(guid, loader);
    if (result.already_exist)
    {
        // TODO. log
        SKR_LOG_WARN(u8"type loader already exist.");
    }
}
void unregister_type_loader(const GUID& guid)
{
    auto result = type_loaders().remove(guid);
    if (!result)
    {
        // TODO. log
        SKR_LOG_WARN(u8"type loader not exist.");
    }
}

// generic type loader
void register_generic_type_loader(const GUID& generic_guid, GenericTypeLoader* loader)
{
}
void unregister_generic_type_loader(const GUID& generic_guid)
{
}

// get type (after register)
Type* get_type_from_guid(const GUID& guid)
{
    auto loaded_result = loaded_types().find(guid);
    if (loaded_result)
    {
        return loaded_result->value;
    }
    else
    {
        auto loader_result = type_loaders().find(guid);
        if (loader_result)
        {
            auto type = loader_result->value->load();
            loaded_types().add(guid, type);
            return type;
        }
    }

    return nullptr;
}
Type* get_type_from_type_desc(Span<TypeDesc> type_desc)
{
    if (type_desc.size() == 1)
    {
        return get_type_from_guid(type_desc[0].value_guid());
    }
    else
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        return nullptr;
    }
}
} // namespace skr::rttr