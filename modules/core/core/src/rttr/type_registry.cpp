#include "SkrRTTR/type_registry.hpp"
#include "SkrContainers/map.hpp"
#include "SkrContainers/multi_map.hpp"
#include "SkrCore/exec_static.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/type.hpp"

#include <mutex>

namespace skr::rttr
{
// static data
static MultiMap<GUID, TypeLoaderFunc>& type_load_funcs()
{
    static MultiMap<GUID, TypeLoaderFunc> s_type_load_funcs;
    return s_type_load_funcs;
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

// auto unload
SKR_EXEC_STATIC_DTOR
{
    unload_all_types();
};

// type register (loader)
void register_type_loader(const GUID& guid, TypeLoaderFunc load_func)
{
    type_load_funcs().add(guid, load_func);
}
void unregister_type_loader(const GUID& guid, TypeLoaderFunc load_func)
{
    auto ref = type_load_funcs().find(guid);
    while (ref.value() != load_func)
    {
        ref = type_load_funcs().find_next(ref, guid);
    }
    if (ref)
    {
        type_load_funcs().remove_at(ref.index());
    }
}
void unregister_all_type_loader(const GUID& guid)
{
    type_load_funcs().remove_all(guid);
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
        auto loader_result = type_load_funcs().find(guid);
        if (loader_result)
        {
            // create type
            auto type = SkrNew<Type>();
            loaded_types().add(guid, type);

            // load type
            while (loader_result)
            {
                loader_result.value()(type);
                loader_result = type_load_funcs().find_next(loader_result, guid);
            }

            // optimize data
            type->build_optimize_data();
            return type;
        }
    }

    return nullptr;
}
void unload_all_types()
{
    std::lock_guard _lock(load_type_mutex());

    // release type memory
    for (auto& type : loaded_types())
    {
        SkrDelete(type.value);
    }

    // clear loaded types
    loaded_types().clear();

    loaded_types().clear();
}

} // namespace skr::rttr