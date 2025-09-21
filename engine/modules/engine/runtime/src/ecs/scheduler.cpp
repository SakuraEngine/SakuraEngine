#include "SkrRuntime/ecs/scheduler.hpp"
#include "SkrCore/async/wait_timeout.hpp"
#include "SkrCore/memory/sp.hpp"
#include "./../sugoi/impl/query.hpp"

namespace skr::ecs
{

EDependencySyncMode TaskSignature::DeterminSyncMode(TaskSignature* t, EAccessMode current, TaskSignature* last_t, EAccessMode last)
{
    if (current == EAccessMode::Seq && last == EAccessMode::Seq)
        return EDependencySyncMode::PerChunk;
    return EDependencySyncMode::WholeTask;
}

inline static bool HasSelfConfictReadWrite(skr::RC<TaskSignature> task, ComponentAccess acess, skr::Span<ComponentAccess> to_search)
{
    if (task->opts && task->opts->no_parallelization)
        return true;
    
    if (acess.mode == EAccessMode::Random)
    {
        for (auto other : to_search)
        {
            if (other.type == acess.type)
                return true; // access conflict to self
        }
    }
    return false;
}

void StaticDependencyAnalyzer::process(skr::RC<TaskSignature> new_task)
{
    SkrZoneScopedN("StaticDependencyAnalyzer::Process");

    StackVector<TaskDependency> _collector;
    bool HasSelfConflict = false;
    for (auto read : new_task->reads)
    {
        HasSelfConflict |= HasSelfConfictReadWrite(new_task, read, new_task->writes);

        auto access = accesses.try_add_default(read.type);
        if (access.already_exist() && access.value().last_writer.first) // RAW
        {
            auto&& [writer, mode] = access.value().last_writer;
            if (!writer->_finish.test())
                _collector.emplace(writer, TaskSignature::DeterminSyncMode(new_task.get(), read.mode, writer.get(), mode));
        }
        access.value().readers.add(new_task, read.mode);
    }
    for (auto write : new_task->writes)
    {
        HasSelfConflict |= HasSelfConfictReadWrite(new_task, write, new_task->reads);

        auto access = accesses.try_add_default(write.type);
        if (access.already_exist() && access.value().readers.size()) // WAR
        {
            for (auto& [reader, mode] : access.value().readers)
            {
                if (reader == new_task)
                    continue; // skip self

                if (!reader->_finish.test())
                    _collector.emplace(reader, TaskSignature::DeterminSyncMode(new_task.get(), write.mode, reader.get(), mode));
            }
        }
        if (access.already_exist() && access.value().last_writer.first) // WAW
        {
            auto&& [writer, mode] = access.value().last_writer;
            if (!writer->_finish.test())
                _collector.emplace(writer, TaskSignature::DeterminSyncMode(new_task.get(), write.mode, writer.get(), mode));
        }
        access.value().last_writer = { new_task, write.mode };
        access.value().readers.clear();
    }
    new_task->_static_dependencies = _collector;
    new_task->self_confict = HasSelfConflict;
}

void WorkUnitGenerator::process(skr::RC<TaskSignature> new_task)
{
    SkrZoneScopedN("WorkUnitGenerator::Process");

    // collect work units
    struct CollectContext
    {
        const sugoi_query_t* query;
        WorkUnitGenerator* _this;
        skr::RC<TaskSignature> new_task;
        skr::task::weak_counter_t last_unit_finish_counter;
        WorkGroup* work_group = nullptr;
        uint64_t total_units = 0;
        uint64_t total_jobs = 0;
    } ctx;
    ctx.query = new_task->query;
    ctx._this = this;
    ctx.new_task = new_task;

    static const auto collect_units = +[](void* usr_data, sugoi_chunk_view_t* view) {
        if (view->count == 0)
            return;

        CollectContext* ctx = (CollectContext*)usr_data;
        auto try_add = ctx->work_group->units.try_add_default(view->chunk);
        auto& unit = try_add.value();
        if (!try_add.already_exist())
        {
            if (ctx->new_task->self_confict && (ctx->total_units != 0))
            {
                unit.dependencies.add(ctx->last_unit_finish_counter);
            }
            ctx->total_units += 1;
            ctx->last_unit_finish_counter = unit.finish;

            for (auto [dependency, mode] : ctx->work_group->dependencies)
            {
                if (mode == EDependencySyncMode::WholeTask)
                {
                    unit.dependencies.add(dependency->_finish_counter);
                }
                else if (mode == EDependencySyncMode::PerChunk)
                {
                    if (auto depend_group = dependency->_work_groups.find(ctx->work_group->group))
                    {
                        if (auto depend_unit = depend_group.value().units.find(view->chunk))
                        {
                            unit.dependencies.add(depend_unit.value().finish);
                        }
                    }
                }
            }
        }
        unit.chunk_views.add(*view);
        unit.finish.add(1);
        ctx->total_jobs += 1;
    };

    if (new_task->_is_run_with)
    {
        static const auto batch_wgp = +[](void* u, sugoi_chunk_view_t* v) -> void {
            auto& ctx = *(CollectContext*)u;
            auto group = v->chunk->group;
            ctx.work_group = &ctx.new_task->_work_groups.try_add_default(group).value();
            ctx.work_group->group = group;
            for (auto dependency : ctx.new_task->_static_dependencies)
            {
                // can't be optimized, usually because of random accesses
                if (dependency.mode == EDependencySyncMode::WholeTask)
                {
                    ctx.work_group->dependencies.add(dependency);
                }
                if (dependency.task->query) // from query, if two systems never operate on a same group. we can skip the denepdnecy
                {
                    if (bool confict = sugoiQ_match_group(dependency.task->query, group))
                    {
                        ctx.work_group->dependencies.add(dependency);
                    }
                }
                else // from workgroup
                {
                    ctx.work_group->dependencies.add(dependency);
                }
            }
            collect_units(&ctx, v);
        };
        sugoiS_batch(new_task->storage,
            (sugoi_entity_t*)new_task->_run_with.data(),
            new_task->_run_with.size(),
            batch_wgp,
            &ctx);
    }
    else
    {
        static const auto filter_group = +[](void* u, sugoi_group_t* group) {
            auto& ctx = *(CollectContext*)u;
            ctx.work_group = &ctx.new_task->_work_groups.try_add_default(group).value();
            ctx.work_group->group = group;
            for (auto dependency : ctx.new_task->_static_dependencies)
            {
                // can't be optimized, usually because of random accesses
                if (dependency.mode == EDependencySyncMode::WholeTask)
                {
                    ctx.work_group->dependencies.add(dependency);
                }
                if (dependency.task->query) // from query, if two systems never operate on a same group. we can skip the denepdnecy
                {
                    if (bool confict = sugoiQ_match_group(dependency.task->query, group))
                    {
                        ctx.work_group->dependencies.add(dependency);
                    }
                }
                else // from workgroup
                {
                    ctx.work_group->dependencies.add(dependency);
                }
            }
            sugoiQ_in_group(ctx.query, group, collect_units, &ctx);
        };
        sugoiQ_get_groups(ctx.query, filter_group, &ctx);
    }
    new_task->_finish_counter.add(ctx.total_jobs);
    ctx.new_task->_work_groups.compact();
}

static skr::UPtr<TaskScheduler> gInstance = nullptr;
static std::atomic_bool gInitialized = false;
static std::mutex gInstanceMutex;
void TaskScheduler::Initialize(const ServiceThreadDesc& desc, skr::task::scheduler_t& scheduler) SKR_NOEXCEPT
{
    std::lock_guard<std::mutex> lock(gInstanceMutex);
    if (gInitialized.load(std::memory_order_acquire))
        return;

    StackAllocator::Initialize();
    gInstance = skr::UPtr<TaskScheduler>::New(desc, scheduler);
    gInitialized.store(true, std::memory_order_release);
}

void TaskScheduler::Finalize() SKR_NOEXCEPT
{
    std::lock_guard<std::mutex> lock(gInstanceMutex);
    if (!gInitialized.load(std::memory_order_acquire))
        return;
    gInstance.reset();
    gInitialized.store(false, std::memory_order_release);

    StackAllocator::Finalize();
}

TaskScheduler* TaskScheduler::Get() SKR_NOEXCEPT
{
    return gInstance.get();
}

TaskScheduler::TaskScheduler(const ServiceThreadDesc& desc, skr::task::scheduler_t& scheduler) SKR_NOEXCEPT
    : AsyncService(desc),
      _scheduler(scheduler)
{
}

TaskScheduler::~TaskScheduler()
{
}

void TaskScheduler::add_task(skr::RC<TaskSignature> task)
{
    {
        _clear_mtx.lock_shared();
        SKR_DEFER({ _clear_mtx.unlock_shared(); });

        _tasks.enqueue(task);
    }

    skr_atomic_fetch_add(&_enqueued_tasks, 1);
    awake();
}

// clang-format off
void TaskScheduler::dispatch(skr::RC<TaskSignature> signature)
{
    const auto& wgps = signature->work_groups();

    for (const auto& [_, work_group] : wgps)
    {
        SkrZoneScopedN("TaskScheduler::DispatchGroup");
        // create a task for each work unit
        for (uint32_t i = 0; i < work_group.units.size(); i++)
        {
            SkrZoneScopedN("TaskScheduler::DispatchUnit");
            const auto& [__, unit] = work_group.units.at(i);
            
            for (auto chunk_view : unit.chunk_views)
            {
                auto batch_size = signature->task->batch_size ? signature->task->batch_size : chunk_view.count;
                batch_size = signature->self_confict ? UINT32_MAX : batch_size; // if self-confict, we can't batch
                batch_size = std::min(batch_size, chunk_view.count);
                const auto batch_count = (chunk_view.count + batch_size - 1) / batch_size;
                running.add(1);
                {
                    SkrZoneScopedN("DispatchUnit::ScheduleTask");
                    skr::task::schedule([
                        this, &unit, chunk_view, signature, batch_count, batch_size,
                        // capture task lifetime into payload body
                        task = signature->task
                    ](){
                        {
                            for (auto dep : unit.dependencies)
                                dep.lock().wait(false);
                        }
                        SKR_DEFER({ 
                            running.decrement(); 
                            unit.finish.decrement(); 
                            signature->_finish_counter.decrement(); 
                            if (signature->_finish_counter.test())
                            {
                                signature->_finish.signal();
                                if (signature->opts)
                                {
                                    for (auto on_finish_counter : signature->opts->on_finishes)
                                        on_finish_counter.lock().signal();
                                }
                            }
                        });
                        if (batch_count == 1)
                        {
                            task->func(chunk_view, chunk_view.count, 0);
                        }
                        else
                        {
                            SkrZoneScopedN("WorkUnit::Batch");
                            skr::task::counter_t batch_counter;
                            batch_counter.add(batch_count);
                            for (uint32_t i = 0; i < batch_count; i += 1)
                            {
                                const auto remain = chunk_view.count - i * batch_size;
                                auto view = chunk_view;
                                const auto count = std::min(remain, batch_size);
                                const auto offset = i * batch_size;
                                skr::task::schedule(
                                [task, view, batch_counter, count, offset]() mutable {
                                    task->func(view, count, offset);
                                    batch_counter.decrement();
                                }, nullptr);
                            }
                            batch_counter.wait(true);
                        }
                    }, nullptr);
                }
            }
        }
    }

    // release the ownership so tasks can free the task from payload
    signature->task.reset(nullptr);
}
// clang-format on

void TaskScheduler::on_run() SKR_NOEXCEPT
{
    _scheduler.bind();
}

void TaskScheduler::on_exit() SKR_NOEXCEPT
{
    _scheduler.unbind();
}

void TaskScheduler::run() SKR_NOEXCEPT
{
    AsyncService::run();
}

AsyncResult TaskScheduler::serve() SKR_NOEXCEPT
{
    skr::RC<TaskSignature> task;
    if (_tasks.try_dequeue(task))
    {
        _analyzer.process(task);
        _generator.process(task);

        dispatch(task);
        skr_atomic_fetch_add(&_enqueued_tasks, -1);

        _dispatched_tasks.add(task);
    }
    else
    {
        sleep();
    }
    return ASYNC_RESULT_OK;
}

void TaskScheduler::flush_all()
{
    while (skr_atomic_load(&_enqueued_tasks) != 0)
        ;
}

void TaskScheduler::sync_all()
{
    _clear_mtx.lock();
    SKR_DEFER({ _clear_mtx.unlock(); });

    flush_all();
    running.wait(true);

    _analyzer.accesses.~StackMap<TypeIndex, StaticDependencyAnalyzer::AccessInfo>();
    _dispatched_tasks.~StackVector<skr::RC<TaskSignature>>();
    _tasks.~StackConcurrentQueue<skr::RC<TaskSignature>>();

    StackAllocator::Reset();

    new (&_analyzer.accesses) StackMap<TypeIndex, StaticDependencyAnalyzer::AccessInfo>();
    new (&_dispatched_tasks) StackVector<skr::RC<TaskSignature>>();
    new (&_tasks) StackConcurrentQueue<skr::RC<TaskSignature>>();
}

void TaskScheduler::stop_and_exit()
{
    flush_all();
    sync_all();

    if (get_status() == skr::ServiceThread::Status::kStatusRunning)
    {
        SKR_LOG_BACKTRACE(u8"runner: request to stop.");
        setServiceStatus(SKR_ASYNC_SERVICE_STATUS_QUITING);
        stop();
    }
    else
    {
        SKR_LOG_BACKTRACE(u8"runner: stop already requested.");
    }

    wait_timeout<u8"WaitTaskScheduler">([&] { return get_status() == skr::ServiceThread::kStatusStopped; });
    exit();
}

} // namespace skr::ecs