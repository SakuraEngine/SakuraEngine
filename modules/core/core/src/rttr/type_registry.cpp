#include "SkrRTTR/type_registry.hpp"
#include "SkrContainers/map.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/type.hpp"

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
        SKR_LOG_FMT_WARN(u8"\"{}\" type loader already exist.", guid);
    }
}
void unregister_type_loader(const GUID& guid)
{
    auto result = type_loaders().remove(guid);
    if (!result)
    {
        SKR_LOG_WARN(u8"\"{}\" type loader not exist.", guid);
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
} // namespace skr::rttr