#include "SkrProfile/profile.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrBase/misc/defer.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrRTTR/type_registry.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrCore/module/module.hpp"
#include "SkrCore/async/thread_job.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrContainers/hashmap.hpp"

namespace skd::asset
{
struct CookSystemImpl : public skd::asset::CookSystem
{
    friend struct ::SkrToolCoreModule;
    using AssetMap = skr::ParallelFlatHashMap<skr::GUID, skr::RC<AssetMetaFile>, skr::Hash<skr::GUID>>;
    using CookingMap = skr::ParallelFlatHashMap<skr::GUID, CookContext*, skr::Hash<skr::GUID>>;

    ~CookSystemImpl()
    {
        for (auto& [k, asset] : assets)
            asset.reset();
        assets.clear();
    }

    Cooker* GetCooker(AssetMetaFile* info) const;
    skr::task::event_t AddCookTask(AssetID asset);
    skr::task::event_t EnsureCooked(AssetID asset) override;

    void WaitForAll() override;
    bool AllCompleted() const override;

    skr::RC<AssetMetaFile> LoadAssetMeta(SProject* project, const URI& uri) override;
    bool ImportAssetMeta(SProject* project, skr::RC<AssetMetaFile> asset, skr::RC<Importer> importer, skr::RC<AssetMetadata> meta) override;
    bool SaveAssetMeta(SProject* project, skr::RC<AssetMetaFile> asset) override;
    skr::RC<AssetMetaFile> GetAssetMetaFile(AssetID guid) const override;

    void ParallelForEachAsset(uint32_t batch, skr::FunctionRef<void(skr::Span<skr::RC<AssetMetaFile>>)> f) override;

    void RegisterCooker(bool isDefault, skr::GUID cooker, skr::GUID type, Cooker* instance) override;
    void UnregisterCooker(skr::GUID type) override;
    skr::io::IRAMService* GetIOService() override;

protected:
    template <class F, class Iter>
    void ParallelFor(Iter begin, Iter end, size_t batch, F f)
    {
        skr::parallel_for(std::move(begin), std::move(end), batch, std::move(f));
    }

    AssetMap assets;
    CookingMap cooking;
    SMutex ioMutex;

    skr::task::counter_t mainCounter;

    skr::FlatHashMap<skr::GUID, Cooker*, skr::Hash<skr::GUID>> defaultCookers;
    skr::FlatHashMap<skr::GUID, Cooker*, skr::Hash<skr::GUID>> cookers;
    skr::io::IRAMService* ioServices[ioServicesMaxCount];
};
} // namespace skd::asset

namespace skd::asset
{
CookSystem* GetCookSystem()
{
    static skd::asset::CookSystemImpl cook_system;
    return &cook_system;
}

void RegisterCookerToSystem(CookSystem* system, bool isDefault, skr::GUID cooker, skr::GUID type, Cooker* instance)
{
    system->RegisterCooker(isDefault, cooker, type, instance);
}

AssetMetadata::AssetMetadata()
{
}

AssetMetadata::~AssetMetadata()
{
}

AssetMetaFile::AssetMetaFile(const URI& _uri)
    : uri(_uri)
{
}

AssetMetaFile::AssetMetaFile(const URI& _uri, skr::GUID _guid, skr::GUID _type, skr::GUID _cooker)
    : uri(_uri)
    , guid(_guid)
    , resource_type(_type)
    , cooker(_cooker)
{
}

void CookSystemImpl::WaitForAll()
{
    mainCounter.wait(true);
}

bool CookSystemImpl::AllCompleted() const
{
    return mainCounter.test();
}

skr::io::IRAMService* CookSystemImpl::GetIOService()
{
    SMutexLock lock(ioMutex);
    static std::atomic_uint32_t cursor = 0;
    cursor = (cursor % ioServicesMaxCount);
    return ioServices[cursor++];
}

skr::task::event_t CookSystemImpl::AddCookTask(AssetID asset)
{
    CookContext* cookContext = nullptr;
    auto assetfile = GetAssetMetaFile(asset);
    // return existed task if it's already cooking
    {
        // clang-format off
        skr::task::event_t existed_task{ nullptr };
        cooking.lazy_emplace_l(asset, 
        [&](const auto& ctx_kv) 
        { 
            existed_task = ctx_kv.second->GetCounter(); 
        }, 
        [&](const CookingMap::constructor& ctor) 
        {
            cookContext = CookContext::Create(assetfile);
            ctor(asset, cookContext); 
        });
        // clang-format on

        if (existed_task)
            return existed_task;
    }
    skr::task::event_t event;
    cookContext->SetCounter(event);
    auto fiberName = skr::format(u8"Fiber{}", assetfile->guid);
    mainCounter.add(1);
    skr::task::schedule(
        [cookContext, ioService = GetIOService()]() {
            auto system = static_cast<CookSystemImpl*>(GetCookSystem());
            const auto metaAsset = cookContext->GetAssetMetaFile();
            auto cooker = system->GetCooker(metaAsset.get());
            SKR_ASSERT(cooker);

            SkrZoneScopedN("CookingTask");
            {
                const auto rtti_type = skr::get_type_from_guid(metaAsset->resource_type);
                const auto cookerTypeName = rtti_type ? rtti_type->name().c_str_raw() : (const char*)u8"UnknownResource";
                const auto assetName = skr::format(u8"Asset: {}", metaAsset->guid);
                const auto assetType = skr::format(u8"TypeGuid: {}", metaAsset->resource_type);
                const auto scopeName = skr::format(u8"Cook.[{}]", (const skr_char8*)cookerTypeName);
                const auto assetURI = skr::format(u8"Asset: {}", metaAsset->uri.c_str());
                ZoneName(scopeName.c_str_raw(), scopeName.size());
                SkrMessage(assetName.c_str_raw(), assetName.size());
                SkrMessage(assetType.c_str_raw(), assetType.size());
                SkrMessage(assetURI.c_str_raw(), assetURI.size());
            }

            SKR_DEFER({
                auto system = static_cast<CookSystemImpl*>(GetCookSystem());
                auto asset = cookContext->GetAssetMetaFile()->guid;
                // system->cooking.erase_if(asset, [](const auto& ctx_kv) { CookContext::Destroy(ctx_kv.second); return true; });
                system->mainCounter.decrement();
            });

            // setup cook context
            cookContext->SetIOService(ioService);
            cookContext->SetCookerVersion(cooker->Version());
            // SKR_ASSERT(iter != system->cookers.end()); // TODO: error handling
            SKR_LOG_INFO(u8"[CookTask] resource %s cook started!", metaAsset->uri.c_str());
            if (cooker->Cook(cookContext))
            {
                // write resource header
                {
                    SKR_LOG_INFO(u8"[CookTask] resource %s cook finished! updating resource metas.", metaAsset->uri.c_str());
                    skr::ArWriteBin writer;
                    cookContext->WriteHeader(writer, cooker);

                    auto resource_vfs = metaAsset->project->GetResourceVFS();
                    auto relative_path = skr::format(u8"{}.rh", metaAsset->guid);
                    auto file = skr_vfs_fopen(resource_vfs, relative_path.u8_str(), SKR_FM_WRITE_BINARY, SKR_FILE_CREATION_ALWAYS_NEW);
                    if (!file)
                    {
                        SKR_LOG_ERROR(u8"[CookTask] failed to write header file for resource %s!", metaAsset->uri.c_str());
                        return;
                    }
                    SKR_DEFER({ skr_vfs_fclose(file); });
                    skr_vfs_fwrite(file, writer.buffer().data(), 0, writer.buffer().size());
                }
                // write resource dependencies
                {
                    SKR_LOG_INFO(u8"[CookTask] resource %s cook finished! updating dependencies.", metaAsset->uri.c_str());

                    auto writer = skr::ArWriteJson::Create();
                    {
                        skr::Archive::ObjectScope scope(writer);
                        SKR_FAST_CHECK(scope.is_success(), );
                        SKR_FAST_CHECK(writer.key_value(u8"importerVersion", cookContext->GetImporterVersion()), );
                        SKR_FAST_CHECK(writer.key_value(u8"cookerVersion", cookContext->GetCookerVersion()), );
                        SKR_FAST_CHECK(writer.key(u8"files"), );
                        {
                            skr::Archive::ArrayScope array_scope(writer);
                            SKR_FAST_CHECK(array_scope.is_success(), );
                            for (auto& source_path : cookContext->GetSourceFiles())
                            {
                                SKR_FAST_CHECK(writer.value(source_path), );
                            }
                        }
                        SKR_FAST_CHECK(writer.key(u8"dependencies"), );
                        {
                            skr::Archive::ArrayScope array_scope(writer);
                            SKR_FAST_CHECK(array_scope.is_success(), );
                            for (auto& dep : cookContext->GetStaticDependencies())
                            {
                                SKR_FAST_CHECK(writer.value(dep), );
                            }
                        }
                    }

                    auto dependency_vfs = metaAsset->project->GetDependencyVFS();
                    auto relative_path = skr::format(u8"{}.d", metaAsset->guid);
                    auto file = skr_vfs_fopen(dependency_vfs, relative_path.u8_str(), SKR_FM_WRITE, SKR_FILE_CREATION_ALWAYS_NEW);
                    if (!file)
                    {
                        SKR_LOG_ERROR(u8"[CookTask] failed to write dependency file for resource %s!", metaAsset->uri.c_str());
                        return;
                    }
                    SKR_DEFER({ skr_vfs_fclose(file); });
                    skr::String jString;
                    writer.write_to_string(jString);
                    skr_vfs_fwrite(file, jString.c_str_raw(), 0, jString.length_buffer());
                }
            }
        },
        &event,
        fiberName.c_str_raw()
    );
    return event;
}

void CookSystemImpl::RegisterCooker(bool isDefault, skr::GUID cooker, skr::GUID type, Cooker* instance)
{
    SKR_ASSERT(instance->system == nullptr);
    instance->system = this;
    cookers.insert(std::make_pair(cooker, instance));
    if (isDefault)
    {
        auto result = defaultCookers.insert(std::make_pair(type, instance));
        SKR_ASSERT(result.second);
        (void)result;
    }
}

void CookSystemImpl::UnregisterCooker(skr::GUID guid)
{
    cookers.erase(guid);
}

Cooker* CookSystemImpl::GetCooker(AssetMetaFile* info) const
{
    if (info->cooker.is_zero())
    {
        auto it = defaultCookers.find(info->resource_type);
        if (it != defaultCookers.end())
            return it->second;
        return nullptr;
    }
    auto it = cookers.find(info->cooker);
    if (it != cookers.end()) return it->second;
    return nullptr;
}

#define SKR_CHECK_RESULT(result, name)                                                                                          \
    if (result.error() != simdjson::SUCCESS)                                                                                    \
    {                                                                                                                           \
        SKR_LOG_INFO(u8"[CookSystemImpl::EnsureCooked] " name " file parse failed! resource guid: %s", metaAsset->uri.c_str()); \
        return false;                                                                                                           \
    }

skr::task::event_t CookSystemImpl::EnsureCooked(AssetID asset)
{
    SkrZoneScoped;
    {
        skr::task::event_t result{ nullptr };
        cooking.if_contains(asset, [&](const auto& ctx_kv) {
            result = ctx_kv.second->GetCounter();
        });
        if (result)
            return result;
    }
    auto metaAsset = GetAssetMetaFile(asset);
    if (!metaAsset)
    {
        SKR_LOG_ERROR(u8"[CookSystemImpl::EnsureCooked] resource not exist! asset path: %s", metaAsset->uri.c_str());
        return nullptr;
    }
    return AddCookTask(asset);
}

skr::RC<AssetMetaFile> CookSystemImpl::LoadAssetMeta(SProject* project, const URI& uri)
{
    SkrZoneScoped;
    skr::String meta_content;
    if (project->LoadAssetMeta(uri.string(), meta_content))
    {
        // read meta file
        auto reader = skr::ArReadJson::ReadBuffer(meta_content.data(), meta_content.size());
        if (reader.is_failed())
        {
            SKR_LOG_ERROR(u8"[CookSystemImpl::LoadAssetMeta] failed to parse metafile: %s", uri.c_str());
            return nullptr;
        }

        /// parse meta file
        auto metafile = skr::RC<AssetMetaFile>::New(uri);
        reader.value(*metafile);
        metafile->project = project;
        metafile->SetContent(std::move(meta_content));
        assets.insert(std::make_pair(metafile->guid, metafile));
        return metafile;
    }
    SKR_ASSERT(false);
    return nullptr;
}

bool CookSystemImpl::ImportAssetMeta(SProject* project, skr::RC<AssetMetaFile> asset, skr::RC<Importer> importer, skr::RC<AssetMetadata> meta)
{
    asset->importer = importer;
    asset->metadata = meta;
    asset->project = project;
    return assets.insert({ asset->GetGUID(), asset }).second;
}

bool CookSystemImpl::SaveAssetMeta(SProject* project, skr::RC<AssetMetaFile> asset)
{
    const auto uri = asset->GetURI();

    auto writer = skr::ArWriteJson::Create();
    writer.value(*asset);
    skr::String content;
    writer.write_to_string(content);
    return project->SaveAssetMeta(uri, content);
}

skr::RC<AssetMetaFile> CookSystemImpl::GetAssetMetaFile(AssetID asset) const
{
    auto it = assets.find(asset);
    if (it != assets.end())
        return it->second;
    return nullptr;
}

void CookSystemImpl::ParallelForEachAsset(uint32_t batch, skr::FunctionRef<void(skr::Span<skr::RC<AssetMetaFile>>)> f)
{
    ParallelFor(assets.begin(), assets.end(), batch, [f, batch](auto begin, auto end) {
        skr::Vector<skr::RC<AssetMetaFile>> records;
        records.reserve(batch);
        for (auto it = begin; it != end; ++it)
        {
            records.add(it->second);
        }
        f(records);
    });
}

} // namespace skd::asset

struct TOOL_CORE_API SkrToolCoreModule : public skr::IDynamicModule
{
    skr::JobQueue* io_job_queue = nullptr;
    skr::JobQueue* io_callback_job_queue = nullptr;
    virtual void on_load(int argc, char8_t** argv) override
    {
        auto cook_system = (skd::asset::CookSystemImpl*)skd::asset::GetCookSystem();
        skr_init_mutex(&cook_system->ioMutex);

        auto jqDesc = make_zeroed<skr::JobQueueDesc>();
        jqDesc.thread_count = 1;
        jqDesc.priority = SKR_THREAD_ABOVE_NORMAL;
        jqDesc.name = u8"Tool-IOJobQueue";
        io_job_queue = SkrNew<skr::JobQueue>(jqDesc);

        jqDesc.name = u8"Tool-IOCallbackJobQueue";
        io_callback_job_queue = SkrNew<skr::JobQueue>(jqDesc);

        for (auto& ioService : cook_system->ioServices)
        {
            // all used up
            if (ioService == nullptr)
            {
                skr_ram_io_service_desc_t desc = {};
                desc.sleep_time = SKR_ASYNC_SERVICE_SLEEP_TIME_MAX;
                desc.awake_at_request = true;
                desc.name = u8"Tool-IOService";
                desc.io_job_queue = io_job_queue;
                desc.callback_job_queue = io_callback_job_queue;
                ioService = skr_io_ram_service_t::create(&desc);
                ioService->run();
            }
        }
    }

    virtual void on_unload() override
    {
        auto cook_system = (skd::asset::CookSystemImpl*)skd::asset::GetCookSystem();
        skr_destroy_mutex(&cook_system->ioMutex);
        for (auto ioService : cook_system->ioServices)
        {
            if (ioService)
                skr_io_ram_service_t::destroy(ioService);
        }

        SkrDelete(io_callback_job_queue);
        SkrDelete(io_job_queue);
    }
};
IMPLEMENT_DYNAMIC_MODULE(SkrToolCoreModule, SkrToolCore);