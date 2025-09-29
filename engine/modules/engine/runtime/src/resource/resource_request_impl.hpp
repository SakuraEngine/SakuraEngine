#pragma once
#include <atomic>
#include "SkrRuntime/io/ram_io.hpp"
#include "SkrRuntime/resource/resource_system.hpp"
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
    skr::Span<const SResourceHandle> GetDependencies() const override;

    void Update() override;

    bool Okay() override;
    bool Failed() override;

protected:
    void _UnloadDependencies();

    std::atomic_bool requestInstall = false;
    ResourceSystem* system;
    ResourceFactory* factory;
    skr_vfs_t* vfs;

    // skr::InlineVector<skr::GUID, 4> dependencies;
    SResourceRecord* resourceRecord;
    skr_io_future_t dataFuture;
    skr::BlobId dataBlob;
    skr::String resourceUrl;

    skr::task::event_t serdeEvent;
    bool serdeScheduled;

    SMutexObject updateMutex;
};
} // namespace skr