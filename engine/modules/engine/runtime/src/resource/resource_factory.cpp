#include "SkrRuntime/resource/resource_factory.h"
#include "SkrRuntime/resource/resource_header.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrRTTR/type_registry.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/export/extern_methods.hpp"

namespace skr
{
bool ResourceFactory::Deserialize(SResourceRecord* record, skr::ArchiveRead* reader)
{
    if (auto type = skr::get_type_from_guid(record->header.type))
    {
        // allocate memory
        void* p_obj = type->alloc();

        // call ctor
        {
            auto found_ctor = type->find_default_ctor();
            if (found_ctor.is_valid())
                found_ctor.invoke(p_obj);
        }

        // read
        {
            auto found_serde_read = type->find_serde_read();
            if (found_serde_read.is_valid())
            {
                found_serde_read.invoke(*reader, p_obj);
            }
        }

        // destroy if failed
        if (reader->is_failed())
        {
            auto dtor = type->dtor_invoker();
            if (dtor)
                dtor(p_obj);
            type->free(p_obj);
        }

        record->resource = p_obj;
        return true;
    }
    SKR_LOG_FMT_ERROR(u8"Failed to deserialize resource of type {}", record->header.type);
    SKR_UNREACHABLE_CODE();
    return false;
}

bool ResourceFactory::Unload(SResourceRecord* record)
{
    record->header.dependencies.clear();
    if (record->destructor)
        record->destructor(record->resource);
#ifdef SKR_RESOURCE_DEV_MODE
    if (record->artifactsDestructor)
        record->artifactsDestructor(record->artifacts);
#endif
    return true;
}

ESkrInstallStatus ResourceFactory::UpdateInstall(SResourceRecord* record)
{
    SKR_UNREACHABLE_CODE();
    return SKR_INSTALL_STATUS_SUCCEED;
}
} // namespace skr