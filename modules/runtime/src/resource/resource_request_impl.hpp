#pragma once
#include "utils/io.h"
#include <EASTL/fixed_vector.h>
#include "resource/resource_system.h"
#include "task/task.hpp"
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
    skr_async_request_t ioRequest;
    skr_async_ram_destination_t ioDestination;
    skr::string resourceUrl;
#ifdef SKR_RESOURCE_DEV_MODE
    skr_async_request_t artifactsIoRequest;
    skr_async_ram_destination_t artifactsIoDestination;
    skr::string artifactsUrl;
#endif
    uint8_t* data;
    uint64_t size;
#ifdef SKR_RESOURCE_DEV_MODE
    uint8_t* artifactsData;
    uint64_t artifactsSize;
#endif
    skr::task::event_t serdeEvent;
    bool serdeScheduled;
    int serdeResult; 

    SMutexObject updateMutex;
    bool dependenciesLoaded = false;
};
} // namespace resource
} // namespace skr