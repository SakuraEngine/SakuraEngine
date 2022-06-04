#include "asset/config_asset.hpp"
#include "asset/cooker.hpp"
#include "asset/importer.hpp"
#include "ftl/fibtex.h"
#include "platform/configure.h"
#include "platform/memory.h"
#include "platform/vfs.h"
#include "resource/config_resource.h"
#include "json/reader.h"
#include <mutex>
#include "platform/debug.h"
#include "simdjson.h"
#include "type/type_registry.h"
#include "utils/log.h"
#include "utils/defer.hpp"
#include "SkrRT/typeid.generated.hpp"
#include "utils/io.hpp"

namespace skd::asset
{
TOOL_API SConfigRegistry* GetConfigRegistry()
{
    static SConfigRegistry registry;
    return &registry;
}
} // namespace skd::asset

namespace skd
{
namespace asset
{
void* SJsonConfigImporter::Import(skr::io::RAMService* ioService, const SAssetRecord* record)
{
    auto registry = GetConfigRegistry();
    auto iter = registry->typeInfos.find(configType);
    if (iter == registry->typeInfos.end())
    {
        SKR_LOG_ERROR("import resource %s failed, type is not registered as config", record->path.u8string().c_str());
        return nullptr;
    }

    auto u8Path = record->path.u8string();
#if 1
    ftl::AtomicFlag counter(&GetCookSystem()->GetScheduler());
    counter.Set();
    skr_ram_io_t ramIO = {};
    ramIO.bytes = nullptr;
    ramIO.offset = 0;
    ramIO.size = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](void* data) noexcept {
        auto pCounter = (ftl::AtomicFlag*)data;
        pCounter->Clear();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&counter;
    skr_async_io_request_t ioRequest = {};
    ioService->request(record->project->vfs, &ramIO, &ioRequest);
    GetCookSystem()->scheduler->WaitForCounter(&counter, true);
    auto jsonString = simdjson::padded_string((char8_t*)ioRequest.bytes, ioRequest.size);
    sakura_free(ioRequest.bytes);
#else
    auto file = skr_vfs_fopen(record->project->vfs, u8Path.c_str(), SKR_FM_READ, SKR_FILE_CREATION_OPEN_EXISTING);
    SKR_DEFER({ skr_vfs_fclose(file); });
    auto size = skr_vfs_fsize(file);
    auto buffer = (char*)sakura_malloc(size + 1);
    skr_vfs_fread(file, buffer, 0, size);
    buffer[size] = 0;
    auto jsonString = simdjson::padded_string(buffer, size);
    sakura_free(buffer);
#endif
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(jsonString);
    skr_config_resource_t* resource = skr::resource::SConfigFactory::NewConfig(configType);
    iter->second.Import(doc.value(), resource->configData);
    return resource; //导入具体数据
}
uint32_t SConfigCooker::Version()
{
    return 0;
}
bool SConfigCooker::Cook(SCookContext* ctx)
{
    //-----load config
    // no cook config for config, skipping
    //-----import resource object
    auto resource = ctx->Import<skr_config_resource_t>();
    SKR_DEFER({ SkrDelete(resource); });
    //-----emit dependencies
    // no static dependencies
    //-----cook resource
    // no cook needed for config, just binarize it
    //-----fetch runtime dependencies
    auto type = (skr::type::RecordType*)skr_get_type(&resource->configType);
    for (auto& field : type->fields)
    {
        if (field.type->Same(skr::type::type_of<skr_resource_handle_t>::get()))
        {
            auto handle = (skr_resource_handle_t*)((char*)resource->configData + field.offset);
            if (handle->is_null())
                continue;
            ctx->AddRuntimeDependency(handle->get_guid());
        }
    }
    //-----write resource header
    eastl::vector<uint8_t> buffer;
    skr::resource::SBinarySerializer archive(buffer);
    ctx->WriteHeader(archive, this);
    //------write resource object
    skr::resource::SConfigFactory::Serialize(*resource, archive);
    //------save resource to disk
    auto file = fopen(ctx->output.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", ctx->record->guid, ctx->record->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, archive.adapter().writtenBytesCount(), file);
    return true;
}

bool SJsonConfigImporterFactory::CanImport(const SAssetRecord* record)
{
    if (record->path.extension() == ".json")
        return true;
    return false;
}
skr_guid_t SJsonConfigImporterFactory::GetResourceType()
{
    return get_type_id_skr_config_resource_t();
}
SImporter* SJsonConfigImporterFactory::CreateImporter(const SAssetRecord* record)
{
    // TODO: invoke user interface?
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}
} // namespace asset
} // namespace skd