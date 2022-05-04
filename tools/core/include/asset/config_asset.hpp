#pragma once
#include "platform/guid.h"
#include "asset/importer.hpp"
#include "platform/configure.h"

namespace skd reflect
{
namespace asset reflect
{
struct reflect attr("guid" : "D5970221-1A6B-42C4-B604-DA0559E048D6")
SJsonConfigImporter final : public SImporter
{
    using SImporter::SImporter;
    void* Import(const SAssetRecord* record) override;
};
struct SJsonConfigImporterFactory final : public SImporterFactory {
    bool CanImport(const SAssetRecord* record) override;
    skr_guid_t GetResourceType() override;
    SImporter* CreateImporter(const SAssetRecord* record) override;
    SImporter* LoadImporter(const SAssetRecord* record, simdjson::ondemand::value&& object) override;
};
} // namespace reflect
} // namespace reflect