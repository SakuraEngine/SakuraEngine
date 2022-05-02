#pragma once
#include "platform/guid.h"
#include "asset/importer.hpp"
#include "platform/configure.h"

namespace skr
{
namespace asset
{
struct reflect
SJsonConfigImporter final : public SImporter {
    void* Import() override;
};
struct SJsonConfigImporterFactory final : public SImporterFactory {
    void CanImport(skr_guid_t assetGuid) override;
    skr_guid_t GetResourceType() override;
    SImporter* CreateImporter(skr_guid_t assetGuid) override;
};
} // namespace asset
} // namespace skr