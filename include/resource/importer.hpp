#pragma once
#include "platform/guid.h"
#include "resource/resource_header.h"
#include "platform/configure.h"

namespace skr
{
struct reflect SImporter {
    skr_guid_t assetGuid;
    virtual void* Import() = 0;
};
struct SImporterFactory {
    virtual void CanImport(skr_guid_t assetGuid) = 0;
    virtual skr_guid_t GetResourceType() = 0;
    virtual SImporter* CreateImporter(skr_guid_t assetGuid) = 0;
};
} // namespace skr