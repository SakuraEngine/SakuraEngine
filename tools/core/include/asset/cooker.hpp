#pragma once
#include "EASTL/vector.h"
#include "EASTL/shared_ptr.h"
#include "platform/thread.h"
#include "resource/resource_header.h"
#include "tool_configure.h"
#include "asset/asset_registry.hpp"
#include "ftl/task_counter.h"
#include "ftl/task_scheduler.h"
#include "platform/configure.h"
#include "platform/guid.h"
#include "ftl/fibtex.h"

struct skr_vfs_t;
namespace skr::io
{
    class RAMService;
}
namespace skd::asset reflect
{
struct SCookSystem;
struct SCooker;
struct SCookContext;
struct TOOL_API SCooker {
    virtual ~SCooker() {}
    virtual uint32_t Version() = 0;
    virtual bool Cook(SCookContext* ctx) = 0;
    SCookSystem* system;
};
struct TOOL_API SCookContext { // context per job
    SAssetRecord* record;
    SCookSystem* system;
    class skr::io::RAMService* ioService;
    eastl::shared_ptr<ftl::TaskCounter> counter;
    ghc::filesystem::path output;
    eastl::vector<skr_guid_t> staticDependencies;
    eastl::vector<skr_guid_t> runtimeDependencies;
    void AddRuntimeDependency(skr_guid_t resource);
    void* AddStaticDependency(skr_guid_t resource);
    void* _Import();
    template <class T>
    T* Import()
    {
        return (T*)_Import();
    }
    template <class S>
    void WriteHeader(S& s, SCooker* cooker)
    {
        skr_resource_header_t header;
        header.guid = record->guid;
        header.type = record->type;
        header.version = cooker->Version();
        header.dependencies.insert(header.dependencies.end(), runtimeDependencies.begin(), runtimeDependencies.end());
        bitsery::serialize(s, header);
    }
};

struct TOOL_API SCookSystem {
    SCookSystem() noexcept;
    ~SCookSystem() noexcept;
    static ftl::TaskScheduler scheduler;
    eastl::shared_ptr<ftl::TaskCounter> AddCookTask(skr_guid_t resource);
    void* CookOrLoad(skr_guid_t resource);
    eastl::shared_ptr<ftl::TaskCounter> EnsureCooked(skr_guid_t resource);
    void RegisterCooker(skr_guid_t type, SCooker* cooker);
    void UnregisterCooker(skr_guid_t type);
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cookers;
    skr::flat_hash_map<skr_guid_t, SCookContext*, skr::guid::hash> cooking;
    static void RegisterGlobalCooker(void (*)(SCookSystem* system));
    ftl::Fibtex taskMutex;

    class skr::io::RAMService* getIOService();
    static constexpr uint32_t ioServicesMaxCount = 32;
    class skr::io::RAMService* ioServices[ioServicesMaxCount];
};
} // namespace skd::assetreflect