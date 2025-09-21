#pragma once
#include <atomic>
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/resource/resource_system.h"
#include "SkrTask/fib_task.hpp"


namespace skr
{
struct SResourceRequestImpl : public ResourceRequest
{
    friend struct ResourceRegistry;
    friend struct ResourceSystemImpl;
public:
    skr::GUID GetGuid() const override;
    skr::Span<const uint8_t> GetData() const override;
#ifdef SKR_RESOURCE_DEV_MODE
    skr::Span<const uint8_t> GetArtifactsData() const override;
#endif
    skr::Span<const skr::GUID> GetDependencies() const override;

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

    ResourceSystem* system;
    ResourceFactory* factory;
    skr_vfs_t* vfs;

    skr::InlineVector<skr::GUID, 4> dependencies;
    SResourceRecord* resourceRecord;
    skr_io_future_t dataFuture;
    skr::BlobId dataBlob;
    skr::String resourceUrl;
#ifdef SKR_RESOURCE_DEV_MODE
    skr_io_future_t artifactsFuture;
    skr::BlobId artifactsBlob;
    skr::String artifactsUrl;
#endif

    skr::task::event_t serdeEvent;
    bool serdeScheduled;
    int serdeResult; 

    SMutexObject updateMutex;
    bool dependenciesLoaded = false;
};
} // namespace skr