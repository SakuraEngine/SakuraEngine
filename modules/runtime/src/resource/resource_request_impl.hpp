#pragma once
#include "SkrRT/io/ram_io.hpp"
#include <EASTL/fixed_vector.h>
#include "SkrRT/resource/resource_system.h"
#include "SkrRT/async/fib_task.hpp"
#include <atomic>

namespace skr
{
namespace resource
{
struct SResourceRequestImpl : public SResourceRequest
{
    friend struct SResourceRegistry;
    friend struct SResourceSystemImpl;
public:
    skr_guid_t GetGuid() const override;
    skr::span<const uint8_t> GetData() const override;
#ifdef SKR_RESOURCE_DEV_MODE
    skr::span<const uint8_t> GetArtifactsData() const override;
#endif
    skr::span<const skr_guid_t> GetDependencies() const override;

    void UpdateLoad(bool requestInstall) override;
    void UpdateUnload() override;
    void Update() override;

    bool Okay() override;
    bool Yielded() override;
    bool Failed() override;
    bool AsyncSerde() override;

    void OnRequestFileFinished() override;
    void OnRequestLoadFinished() override;

    void LoadTask() override;
protected:
    void _LoadDependencies() override;
    void _UnloadDependencies() override;
    void _LoadFinished() override;
    void _InstallFinished() override;
    void _UnloadResource() override;

    ESkrLoadingPhase currentPhase;
    std::atomic_bool isLoading;
    std::atomic_bool requireLoading;
    std::atomic_bool requestInstall;

    SResourceSystem* system;
    SResourceFactory* factory;
    skr_vfs_t* vfs;

    eastl::fixed_vector<skr_guid_t, 4> dependencies;
    skr_resource_record_t* resourceRecord;
    skr_io_future_t dataFuture;
    skr::BlobId dataBlob;
    skr::string resourceUrl;
#ifdef SKR_RESOURCE_DEV_MODE
    skr_io_future_t artifactsFuture;
    skr::BlobId artifactsBlob;
    skr::string artifactsUrl;
#endif

    skr::task::event_t serdeEvent;
    bool serdeScheduled;
    int serdeResult; 

    SMutexObject updateMutex;
    bool dependenciesLoaded = false;
};
} // namespace resource
} // namespace skr