#pragma once
#include "SkrCore/log.hpp"
#include "SkrCore/blob.hpp"
#include "SkrCore/platform/vfs.h"
#include "SkrContainersDef/function_ref.hpp"
#include "SkrContainers/vector.hpp"
#include "SkrCore/serialize/binary_archive.hpp"
#include "SkrRuntime/resource/resource_header.hpp"
#include "SkrToolCore/cook_system/importer.hpp"
#include "SkrToolCore/cook_system/cooker.hpp"
#include "SkrToolCore/cook_system/asset_meta.hpp"
#include <SkrContainers/function_ref.hpp>

SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service);

namespace skr
{
using AssetID = skr::GUID;
using ResourceID = skr::GUID;

struct TOOL_CORE_API CookContext
{
    friend struct CookSystem;
    friend struct CookSystemImpl;

public:
    virtual ~CookContext() = default;
    virtual Importer* GetImporter() const = 0;
    virtual skr::GUID GetImporterType() const = 0;
    virtual uint32_t GetImporterVersion() const = 0;
    virtual uint32_t GetCookerVersion() const = 0;
    virtual skr::RC<AssetMetaFile> GetAssetMetaFile() = 0;
    virtual skr::RC<const AssetMetaFile> GetAssetMetaFile() const = 0;
    virtual skr::String GetAssetPath() const = 0;

    virtual URI AddSourceFile(const URI& path) = 0;
    virtual URI AddSourceFileAndLoad(skr::io::IRAMService* ioService, const URI& path, skr::BlobId& destination) = 0;
    virtual skr::Span<const URI> GetSourceFiles() const = 0;

    virtual void AddRuntimeDependency(ResourceID resource) = 0;
    virtual void AddSoftRuntimeDependency(ResourceID resource) = 0;
    virtual uint32_t AddStaticDependency(ResourceID resource, bool install) = 0;

    virtual skr::Span<const ResourceID> GetRuntimeDependencies() const = 0;
    virtual skr::Span<const SResourceHandle> GetStaticDependencies() const = 0;
    virtual const SResourceHandle& GetStaticDependency(uint32_t index) const = 0;

    virtual const skr::task::event_t& GetCounter() = 0;

    template <class T>
    T* Import() { return (T*)_Import(); }

    template <class T>
    void Destroy(T* ptr)
    {
        if (ptr) _Destroy(ptr);
    }

    template <class T>
    bool Save(T& resource)
    {
        auto record = GetAssetMetaFile();
        //------save resource to disk
        auto resource_vfs = record->GetProject()->GetResourceVFS();
        auto filename = skr::format(u8"{}.bin", record->GetGUID());
        auto file = skr_vfs_fopen(resource_vfs, filename.u8_str(), SKR_FM_WRITE_BINARY, SKR_FILE_CREATION_ALWAYS_NEW);
        if (!file)
        {
            SKR_LOG_FMT_ERROR(u8"[ConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", record->GetGUID(), record->GetURI().string());
            return false;
        }
        SKR_DEFER({ skr_vfs_fclose(file); });
        //------write resource object
        skr::ArWriteBin writer;
        writer.value(resource);
        if (writer.is_failed()) [[unlikely]]
        {
            SKR_LOG_FMT_ERROR(
                u8"[ConfigCooker::Cook] failed to serialize resource {}! path: {}",
                record->GetGUID(),
                record->GetURI().string()
            );
            return false;
        }
        auto write_size = skr_vfs_fwrite(file, writer.buffer().data(), 0, writer.buffer().size());
        if (write_size != writer.buffer().size())
        {
            SKR_LOG_FMT_ERROR(u8"[ConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", record->GetGUID(), record->GetURI().string());
            return false;
        }
        return true;
    }

    bool SaveExtra(skr::Span<const uint8_t> data, const char8_t* filename)
    {
        auto record = GetAssetMetaFile();
        //------save extra file to disk
        auto resource_vfs = record->GetProject()->GetResourceVFS();
        auto file = skr_vfs_fopen(resource_vfs, filename, SKR_FM_WRITE_BINARY, SKR_FILE_CREATION_ALWAYS_NEW);
        if (!file)
        {
            SKR_LOG_FMT_ERROR(u8"[CookContext::SaveExtra] failed to create file: {}", filename);
            return false;
        }
        SKR_DEFER({ skr_vfs_fclose(file); });

        //------write data
        if (skr_vfs_fwrite(file, data.data(), 0, data.size()) != data.size())
        {
            SKR_LOG_FMT_ERROR(u8"[CookContext::SaveExtra] failed to write data to file: {}", filename);
            return false;
        }
        return true;
    }

protected:
    static CookContext* Create(skr::RC<AssetMetaFile>);
    static void Destroy(CookContext* ctx);

    virtual void SetCounter(skr::task::event_t&) = 0;
    virtual void SetIOService(skr::io::IRAMService*) = 0;
    virtual void SetCookerVersion(uint32_t version) = 0;

    virtual void* _Import() = 0;
    virtual void _Destroy(void*) = 0;

    void WriteHeader(skr::ArchiveWrite& w, Cooker* cooker)
    {
        auto record = GetAssetMetaFile();
        SResourceHeader header;
        header.guid = record->GetGUID();
        header.type = record->GetResourceType();
        header.version = cooker->Version();
        const auto runtime_deps = GetRuntimeDependencies();
        header.dependencies.append(runtime_deps.data(), runtime_deps.size());
        w.value(header);
    }
};

struct TOOL_CORE_API CookSystem
{ // system
public:
    CookSystem() SKR_NOEXCEPT = default;
    virtual ~CookSystem() SKR_NOEXCEPT = default;

    virtual void Initialize() {}
    virtual void Shutdown() {}

    virtual skr::task::event_t EnsureCooked(AssetID asset) = 0;
    virtual void WaitForAll() = 0;
    virtual bool AllCompleted() const = 0;

    virtual skr::RC<AssetMetaFile> LoadAssetMeta(SProject* project, const URI& uri) = 0;
    virtual bool ImportAssetMeta(SProject* project, skr::RC<AssetMetaFile> asset, skr::RC<Importer> importer, skr::RC<AssetMetadata> meta = nullptr) = 0;
    virtual bool SaveAssetMeta(SProject* project, skr::RC<AssetMetaFile> asset) = 0;
    virtual skr::RC<AssetMetaFile> GetAssetMetaFile(AssetID asset) const = 0;

    virtual void ParallelForEachAsset(uint32_t batch, skr::FunctionRef<void(skr::Span<skr::RC<AssetMetaFile>>)> f) = 0;

    virtual void RegisterCooker(bool isDefault, skr::GUID cooker, skr::GUID type, Cooker* instance) = 0;
    virtual void UnregisterCooker(skr::GUID type) = 0;
    virtual skr::io::IRAMService* GetIOService() = 0;

    static constexpr uint32_t ioServicesMaxCount = 1;
};
} // namespace skr
