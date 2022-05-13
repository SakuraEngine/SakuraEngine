#pragma once
#include "EASTL/vector.h"
#include "EASTL/shared_ptr.h"
#include "ftl/task.h"
#include "platform/memory.h"
#include "platform/thread.h"
#include "resource/resource_header.h"
#include "tool_configure.h"
#include "ftl/task_counter.h"
#include "ftl/task_scheduler.h"
#include "platform/configure.h"
#include "platform/guid.h"
#include "ftl/fibtex.h"
#include "utils/hashmap.hpp"
#include "ghc/filesystem.hpp"
#include "simdjson.h"

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

struct TOOL_API SProject {
    ghc::filesystem::path assetPath;
    ghc::filesystem::path outputPath;
    ghc::filesystem::path dependencyPath;
    skr_vfs_t* vfs = nullptr;
    ~SProject() noexcept;
};

struct SAssetRecord {
    skr_guid_t guid;
    skr_guid_t type;
    ghc::filesystem::path path;
    SProject* project;
    simdjson::padded_string meta;
};

struct TOOL_API SCooker {
    virtual ~SCooker() {}
    virtual uint32_t Version() = 0;
    virtual bool Cook(SCookContext* ctx) = 0;
    SCookSystem* system;
};
struct TOOL_API SCookContext { // context per job
    SAssetRecord* record;
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
    void Initialize();
    void Shutdown();
    eastl::shared_ptr<ftl::TaskCounter> AddCookTask(skr_guid_t resource);
    void* CookOrLoad(skr_guid_t resource);
    eastl::shared_ptr<ftl::TaskCounter> EnsureCooked(skr_guid_t resource);
    void RegisterCooker(skr_guid_t type, SCooker* cooker);
    void UnregisterCooker(skr_guid_t type);
    ftl::TaskScheduler& GetScheduler();
    void WaitForAll();
    skr::flat_hash_map<skr_guid_t, SCooker*, skr::guid::hash> cookers;
    using CookingMap = skr::parallel_flat_hash_map<skr_guid_t, SCookContext*, skr::guid::hash>;
    CookingMap cooking;
    SMutex ioMutex;
    class skr::io::RAMService* getIOService();
    static constexpr uint32_t ioServicesMaxCount = 4;
    class skr::io::RAMService* ioServices[ioServicesMaxCount];
    ftl::TaskScheduler* scheduler = nullptr;
    ftl::TaskCounter* mainCounter = nullptr;

    SAssetRecord* GetAssetRecord(const skr_guid_t& guid);
    SAssetRecord* ImportAsset(SProject* project, ghc::filesystem::path path);
    skr::flat_hash_map<skr_guid_t, SAssetRecord*, skr::guid::hash> assets;
    SMutex assetMutex;

    template <class F, class Iter>
    void ParallelFor(Iter begin, Iter end, size_t batch, F f)
    {
        SKR_ASSERT(scheduler != nullptr);
        struct Payload {
            F* f;
            Iter begin;
            Iter end;
        };
        typename std::iterator_traits<Iter>::difference_type n = std::distance(begin, end);
        size_t batchCount = (n / batch) + 1;
        auto payloads = (Payload*)sakura_malloc(batchCount * sizeof(Payload));
        ftl::Task* tasks = (ftl::Task*)sakura_malloc(batchCount * sizeof(ftl::Task));
        auto body = +[](ftl::TaskScheduler* task, void* data) {
            auto payload = (Payload*)data;
            (*payload->f)(payload->begin, payload->end);
        };
        for (size_t i = 0; i < batchCount; ++i)
        {
            payloads[i].f = &f;
            payloads[i].begin = begin;
            auto toAdvance = std::min((size_t)n, batch);
            std::advance(begin, toAdvance);
            n -= toAdvance;
            payloads[i].end = begin;
            tasks[i] = { body, &payloads[i] };
        }
        ftl::TaskCounter counter(scheduler);
        scheduler->AddTasks(batchCount, tasks, ftl::TaskPriority::Normal, &counter);
        sakura_free(tasks);
        scheduler->WaitForCounter(&counter);
        sakura_free(payloads);
    }
};
TOOL_API SCookSystem* GetCookSystem();
} // namespace skd::assetreflect