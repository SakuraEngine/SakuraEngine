#include "resource/resource_factory.h"
#include "resource/resource_header.hpp"
#include "platform/debug.h"

namespace skr
{
namespace resource
{

bool SResourceFactory::Unload(skr_resource_record_t* record)
{
    record->header.dependencies.clear();
    DestroyResource(record);
    return true;
}

void SResourceFactory::DestroyResource(skr_resource_record_t* record)
{
    if (record->destructor)
        record->destructor(record->resource);
}

ESkrLoadStatus SResourceFactory::UpdateLoad(skr_resource_record_t* record)
{
    SKR_UNREACHABLE_CODE();
    return SKR_LOAD_STATUS_SUCCEED;
}

ESkrInstallStatus SResourceFactory::UpdateInstall(skr_resource_record_t* record)
{
    SKR_UNREACHABLE_CODE();
    return SKR_INSTALL_STATUS_SUCCEED;
}
} // namespace resource
} // namespace skr