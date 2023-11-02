#include "SkrRT/resource/resource_factory.h"
#include "SkrRT/resource/resource_header.hpp"
#include "SkrRT/platform/debug.h"
#include "SkrRT/rttr/type_registry.hpp"
#include "SkrRT/rttr/type/type.hpp"
#include "SkrRT/misc/log.hpp"

namespace skr
{
namespace resource
{

int SResourceFactory::Deserialize(skr_resource_record_t* record, skr_binary_reader_t* reader)
{
    if (auto type = skr::rttr::get_type_from_guid(record->header.type))
    {
        auto p_obj = sakura_malloc_aligned(type->size(), type->alignment());
        type->call_ctor(p_obj);
        auto serde_result = type->read_binary(p_obj, reader);
        if (serde_result != 0)
        {
            type->call_dtor(p_obj);
            sakura_free_aligned(p_obj, type->alignment());
            p_obj = nullptr;
        }

        record->resource = p_obj;
        return serde_result;
    }
    SKR_LOG_FMT_ERROR(u8"Failed to deserialize resource of type {}", record->header.type);
    SKR_UNREACHABLE_CODE();
    return 0;
}

bool SResourceFactory::Unload(skr_resource_record_t* record)
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

ESkrInstallStatus SResourceFactory::UpdateInstall(skr_resource_record_t* record)
{
    SKR_UNREACHABLE_CODE();
    return SKR_INSTALL_STATUS_SUCCEED;
}
} // namespace resource
} // namespace skr