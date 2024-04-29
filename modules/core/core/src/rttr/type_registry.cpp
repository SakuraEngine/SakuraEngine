#include "SkrRTTR/type_registry.hpp"
#include "SkrContainers/map.hpp"
#include "SkrCore/log.h"
#include "SkrRTTR/type_loader/type_loader.hpp"
#include "SkrRTTR/type_loader/generic_type_loader.hpp"
#include "SkrRTTR/type/type.hpp"

namespace skr::rttr
{
static Map<GUID, TypeLoader*>& type_loaders()
{
    static Map<GUID, TypeLoader*> s_type_loaders;
    return s_type_loaders;
}
static Map<GUID, Type*>& loaded_types()
{
    static Map<GUID, Type*> s_types;
    return s_types;
}
static Map<GUID, GenericTypeLoader*>& generic_type_loader()
{
    static Map<GUID, GenericTypeLoader*> s_generic_type_loaders;
    return s_generic_type_loaders;
}

static auto& load_type_mutex()
{
    static std::recursive_mutex s_load_type_mutex;
    return s_load_type_mutex;
}

// type register (loader)
void register_type_loader(const GUID& guid, TypeLoader* loader)
{
    auto result = type_loaders().add(guid, loader);
    if (result.already_exist())
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
    auto result = generic_type_loader().add(generic_guid, loader);
    if (result.already_exist())
    {
        // TODO. log
        SKR_LOG_WARN(u8"generic type loader already exist.");
    }
}
void unregister_generic_type_loader(const GUID& generic_guid)
{
    auto result = generic_type_loader().remove(generic_guid);
    if (!result)
    {
        // TODO. log
        SKR_LOG_WARN(u8"generic type loader not exist.");
    }
}

// get type (after register)
Type* get_type_from_guid(const GUID& guid)
{
    std::lock_guard _lock(load_type_mutex());

    auto loaded_result = loaded_types().find(guid);
    if (loaded_result)
    {
        return loaded_result.value();
    }
    else
    {
        auto loader_result = type_loaders().find(guid);
        if (loader_result)
        {
            auto type = loader_result.value()->create();
            loaded_types().add(guid, type);
            loader_result.value()->load(type);
            return type;
        }
    }

    return nullptr;
}

Type* get_type_from_type_desc(span<TypeDesc> type_desc)
{
    std::lock_guard _lock(load_type_mutex());

    if (type_desc.size() == 1)
    {
        return get_type_from_guid(type_desc[0].value_guid());
    }
    else
    {
        if (type_desc[0].type() == SKR_TYPE_DESC_TYPE_TYPE_ID)
        {
            auto result = generic_type_loader().find(type_desc[0].value_guid());
            // TODO. generic 类型查重，最好让 GenericTypeLoader 自己进行查重以提高效率
            if (result)
            {
                auto type = result.value()->load(type_desc);
                loaded_types().add(type->type_id(), type);
                return type;
            }
        }
        else
        {
            SKR_LOG_ERROR(u8"invalid generic type desc, first type desc must be type guid.");
        }
        return nullptr;
    }
}
} // namespace skr::rttr