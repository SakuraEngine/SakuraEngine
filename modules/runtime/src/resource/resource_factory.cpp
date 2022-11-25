#include "resource/resource_factory.h"
#include "resource/resource_header.hpp"
#include "platform/debug.h"

namespace skr
{
namespace resource
{

int SResourceFactory::Deserialize(skr_resource_record_t* record, skr_binary_reader_t* reader)
{
    int serdeResult;
    if (auto type = skr_get_type(&record->header.type))
    {
        auto obj = type->Malloc();
        type->Construct(obj, nullptr, 0);
        serdeResult = type->Deserialize(obj, reader);
        if (serdeResult != 0)
        {
            type->Destruct(obj);
            type->Free(obj);
            obj = nullptr;
        }
        record->resource = obj;
        return serdeResult;
    }
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