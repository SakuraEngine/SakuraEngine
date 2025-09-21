#pragma once
#include "SkrContainersDef/concurrent_queue.hpp"
#include "SkrContainersDef/stl_function.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrCore/memory/rc.hpp"
#include "SkrCore/async/async_service.h"
#include "SkrRuntime/ecs/component.hpp"
#include "SkrRuntime/ecs/stack_allocator.hpp"

namespace skr::ecs
{

struct Task;
struct TaskSignature;
struct StaticDependencyAnalyzer;

enum class EDependencySyncMode
{
    PerChunk,
    WholeTask
};

struct TaskDependency
{
    skr::RC<TaskSignature> task;
    EDependencySyncMode mode;
};

struct WorkUnit
{
    mutable StackVector<sugoi_chunk_view_t> chunk_views;
    mutable skr::task::counter_t finish;
    StackVector<skr::task::weak_counter_t> dependencies;
};
static_assert(sizeof(WorkUnit) <= sizeof(uint64_t) * 8, "better cacheline for WorkUnit");

struct WorkGroup
{
    const sugoi_group_t* group;
    StackMap<const sugoi_chunk_t*, WorkUnit> units;
    StackVector<TaskDependency> dependencies;
};

struct Task
{
    SKR_RC_IMPL();
    using Type = skr::stl_function<void(sugoi_chunk_view_t, uint32_t count, uint32_t offset)>;

    Task::Type func;
    uint32_t batch_size = 0;
};

// Seq Read/Write: 会使用正常的 ChunkView 粒度去同步优化
// Random Read/Write: 会禁用 ChunkView 粒度去同步优化（或许后续可以支持 Query 查重的有限去同步？）
// Environment: 会针对访问的 Env 类型进行 DependencyLevel 计算并禁用去同步优化
// Message Send/Receive: 待定，因为未知全局消息消费的调度形式 
enum class EAccessMode : uint8_t {
    Seq = 0x0,
    Random = 0x1
};
inline constexpr EAccessMode operator|(EAccessMode a, EAccessMode b) {
    return static_cast<EAccessMode>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
inline constexpr EAccessMode operator&(EAccessMode a, EAccessMode b) {
    return static_cast<EAccessMode>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
inline constexpr bool has_flag(EAccessMode a, EAccessMode b) {
    return (a & b) == b;
}

enum class EAccessType : uint8_t {
    Component,
    Environment,
    Message
};

struct ComponentAccess
{
    TypeIndex type;
    EAccessMode mode;
    EAccessType access_type = EAccessType::Component;
};

struct TaskOptions
{
    skr::InlineVector<skr::task::weak_event_t, 4> on_finishes;
    bool no_parallelization = false;
};

struct TaskSignature
{
public:
    SKR_RC_IMPL();
    virtual ~TaskSignature() = default;

    const auto& work_groups() const { return _work_groups; }

    skr::RC<Task> task;
    StackVector<ComponentAccess> reads;
    StackVector<ComponentAccess> writes;
    const sugoi_query_t* query = nullptr;
    uint32_t thread_affinity = ~0;
    bool self_confict = false;
    skr::Optional<TaskOptions> opts;

protected:
    friend struct TaskScheduler;
    friend struct StaticDependencyAnalyzer;
    static EDependencySyncMode DeterminSyncMode(TaskSignature* t, EAccessMode current, TaskSignature* last_t, EAccessMode last);
    StackVector<TaskDependency> _static_dependencies;

    bool _is_run_with = false;
    StackVector<skr::ecs::Entity> _run_with;
    sugoi_storage_t* storage = nullptr;

    friend struct WorkUnitGenerator;
    StackMap<const sugoi_group_t*, WorkGroup> _work_groups;
    skr::task::counter_t _finish_counter;
    skr::task::event_t _finish;

    std::atomic_uint32_t _exec_counter = 0;
};

struct SKR_RUNTIME_API StaticDependencyAnalyzer
{
    void process(skr::RC<TaskSignature> task);

    struct AccessInfo
    {
        std::pair<skr::RC<TaskSignature>, EAccessMode> last_writer;
        StackMap<skr::RC<TaskSignature>, EAccessMode> readers;
    };
    StackMap<TypeIndex, AccessInfo> accesses;
};

struct SKR_RUNTIME_API WorkUnitGenerator
{
    void process(skr::RC<TaskSignature> task);
};

struct SKR_RUNTIME_API TaskScheduler : protected AsyncService
{
public:
    static void Initialize(const ServiceThreadDesc& desc, skr::task::scheduler_t& scheduler) SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;
    static TaskScheduler* Get() SKR_NOEXCEPT;

    void run() SKR_NOEXCEPT;
    void flush_all();
    void sync_all();
    void stop_and_exit();

    TaskScheduler(const ServiceThreadDesc& desc, skr::task::scheduler_t& scheduler) SKR_NOEXCEPT;
    ~TaskScheduler();

protected:
    AsyncResult serve() SKR_NOEXCEPT override;
    void on_run() SKR_NOEXCEPT override;
    void on_exit() SKR_NOEXCEPT override;
    void dispatch(skr::RC<TaskSignature> task);
    
    friend struct ECSWorld;
    void add_task(skr::RC<TaskSignature> task);
    skr::task::counter_t running;

    friend struct StaticDependencyAnalyzer;
    friend struct WorkUnitGenerator;
    SAtomicU32 _enqueued_tasks = 0;
    StackConcurrentQueue<skr::RC<TaskSignature>> _tasks;
    skr::shared_atomic_mutex _clear_mtx;
    StackVector<skr::RC<TaskSignature>> _dispatched_tasks;

    StaticDependencyAnalyzer _analyzer;
    WorkUnitGenerator _generator;
    skr::task::scheduler_t& _scheduler;
};

} // namespace skr::ecs