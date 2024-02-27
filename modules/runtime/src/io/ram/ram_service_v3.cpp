#include "ram_service_v3.hpp"

namespace skr::io
{

uint32_t RAMServiceV3::global_idx = 0;
RAMServiceV3::RAMServiceV3(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
    : name((desc && desc->name) ? skr::String(desc->name) : skr::format(u8"RAMServiceV3-{}", global_idx++))
{
    ::memset((void*)&tick_counts[0], 0, sizeof(tick_counts));

    request_pool    = SmartPoolPtr<RAMRequestMixin, IBlocksRAMRequest>::Create(kIOPoolObjectsMemoryName);
    ram_buffer_pool = SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer>::Create(kIOPoolObjectsMemoryName);
    ram_batch_pool  = SmartPoolPtr<RAMIOBatch, IIOBatch>::Create(kIOPoolObjectsMemoryName);

    actions.emplace(u8"raw_allocate", [](IOBatchId& batch) {
               return true;
           })
    .ref()
    .as_batch_action()
    .exist_and_equal<&RAMState::ram_status>(ERamStatus::SizeAcquired)
    .add_effect<&RAMState::ram_status>(ERamStatus::Allocated);

    actions.emplace(u8"vfs_fopen", [](IOBatchId& batch) {
               return true;
           })
    .ref()
    .as_batch_action()
    .with_flag<&RAMState::prefer_flags>(EPreferFlag::PreferVFS)
    .add_effect<&RAMState::ram_status>(ERamStatus::SizeAcquired)
    .add_effect<&RAMState::file_status>(EFileStatus::Opened);

    actions.emplace(u8"vfs_seek(sync)", [](IOBatchId& batch) {
               return true;
           })
    .ref()
    .as_batch_action()
    .with_flag<&RAMState::prefer_flags>(EPreferFlag::PreferVFS)
    .with_flag<&RAMState::file_status>(EFileStatus::Opened)
    .none<&RAMState::block_status>()
    .add_effect<&RAMState::block_status>(EBlockStatus::Seeked);

    actions.emplace(u8"vfs_read(sync)", [](IOBatchId& batch) {
               return true;
           })
    .ref()
    .as_batch_action()
    .with_flag<&RAMState::prefer_flags>(EPreferFlag::PreferVFS)
    .with_flag<&RAMState::file_status>(EFileStatus::Opened)
    .exist_and_equal<&RAMState::ram_status>(ERamStatus::Allocated)
    .exist_and_equal<&RAMState::block_status>(EBlockStatus::Seeked)
    .add_effect<&RAMState::block_status>(EBlockStatus::Readed);
}

IIOServiceV3* IRAMServiceV3::create(const RAMServiceDescriptor* desc) SKR_NOEXCEPT
{
    return SkrNew<RAMServiceV3>(desc);
}

void IRAMServiceV3::destroy(IIOServiceV3* service) SKR_NOEXCEPT
{
    SkrZoneScopedN("RAMService::destroy");

    auto S = static_cast<RAMServiceV3*>(service);
    SkrDelete(S);
}

IOBatchId RAMServiceV3::open_batch(uint64_t n) SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&batch_sequence, 1);
    return skr::static_pointer_cast<IIOBatch>(ram_batch_pool->allocate((IIOService*)this, seq, n));
}

BlocksRAMRequestId RAMServiceV3::open_request() SKR_NOEXCEPT
{
    uint64_t seq = (uint64_t)skr_atomicu64_add_relaxed(&request_sequence, 1);
    return skr::static_pointer_cast<IBlocksRAMRequest>(request_pool->allocate((IIOService*)this, seq));
}

void RAMServiceV3::request(IOBatchId batch) SKR_NOEXCEPT
{
    const auto priority = batch->get_priority();
    loading_batches[priority].enqueue(batch);
    skr_atomicu64_add_relaxed(&tick_counts[SKR_IO_TICK_STAGE_LOAD], 1);
}

RAMIOBufferId RAMServiceV3::request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT
{
    auto batch  = open_batch(1);
    auto result = batch->add_request(request, future);
    auto buffer = skr::static_pointer_cast<RAMIOBuffer>(result);
    batch->set_priority(priority);
    this->request(batch);
    return buffer;
}

void RAMServiceV3::tick(ESkrIOTickStage type) SKR_NOEXCEPT
{
    IOBatchId batch;
    for (uint32_t priority = SKR_ASYNC_SERVICE_PRIORITY_URGENT; priority < SKR_ASYNC_SERVICE_PRIORITY_COUNT; priority++)
    {
        if (auto success = loading_batches[priority].try_dequeue(batch))
            break;
    }
    if (batch)
    {
        // ensure plan
        RAMInitAndGoal init_goal;
        init_goal.init = RAMWorldState()
                         .set<&RAMState::prefer_flags>(EPreferFlag::PreferVFS)
                         .set<&RAMState::batch_status>(EBatchStatus::Combined);
        init_goal.goal = RAMWorldState()
                         .set<&RAMState::ram_status>(ERamStatus::Allocated)
                         .set<&RAMState::block_status>(EBlockStatus::Readed);
        if (!plans.contains(init_goal))
        {
            Planner planner;
            auto    plan     = planner.plan<true>(init_goal.init, init_goal.goal, actions);
            auto    the_plan = SPtr<PlanType>::Create();
            *the_plan        = std::move(plan);
            plans.insert(init_goal, the_plan);
        }
        const auto& the_plan = plans[init_goal];
        // locate action
        auto RAMBatch = skr::static_pointer_cast<RAMIOBatch>(batch);
        if (RAMBatch->action_index == -1)
            RAMBatch->action_index = 0;
        const auto& step   = the_plan->at(RAMBatch->action_index);
        const auto& action = step.first;
        const auto& state  = step.second;
        // execute action
        auto RAMRequests = RAMBatch->get_requests();
    }
}

uint64_t RAMServiceV3::remaining_count(ESkrIOTickStage type) SKR_NOEXCEPT
{
    return skr_atomicu64_load_acquire(&tick_counts[type]);
}

} // namespace skr::io