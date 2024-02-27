#pragma once
#include "ram_batch.hpp"
#include "ram_request.hpp"
#include "ram_buffer.hpp"
#include "SkrRT/goap/planner.hpp"
#include "SkrBase/containers/lru/lru.hpp"

namespace skr {
namespace io {

enum class EBatchStatus
{
    Combined,
    Split
};

enum class EFileStatus
{
    Closed,
    Opened
};

enum class ERamStatus
{
    SizeAcquired,
    Allocated,
    Freed
};

enum class EBlockStatus
{
    Seeking,
    Seeked,
    Reading,
    Readed
};

enum class ECancelStatus
{
    Cancelling,
    Cancelled
};

enum class EPreferFlag
{
    PreferVFS = 0x0000'0001,
    PreferDStorage = 0x0000'0002,
};
using PreferFlags = uint32_t;

struct RAMState {
    goap::Atom<EBatchStatus, u8"batch_status"> batch_status;
    goap::Atom<ERamStatus, u8"ram_status"> ram_status;
    goap::Atom<EFileStatus, u8"file_status"> file_status;
    goap::Atom<EBlockStatus, u8"block_status"> block_status;
    goap::Atom<ECancelStatus, u8"cancel_status"> cancel_status;
    goap::Atom<PreferFlags, u8"prefer_flags"> prefer_flags;
};
using RAMWorldState = goap::StaticWorldState<RAMState, u8"RAMState">;

struct RAMServiceAction : public goap::Action<RAMWorldState>
{
    RAMServiceAction(const char8_t* name, skr::stl_function<bool(IOBatchId&)> act) SKR_NOEXCEPT
        : skr::goap::Action<RAMWorldState>(name, 1u), action(act)
    {
    }
    RAMServiceAction& as_split_action()
    {
        add_effect<&RAMState::batch_status>(EBatchStatus::Split);
        return *this;
    }
    RAMServiceAction& as_batch_action()
    {
        exist_and_equal<&RAMState::batch_status>(EBatchStatus::Combined);
        return *this;
    }
    skr::stl_function<bool(IOBatchId&)> action;
};

struct RAMServiceV3 final : public IRAMServiceV3
{
    using Planner = goap::Planner<RAMWorldState, RAMServiceAction>;
    using PlanType = Planner::PlanType<true>;

    RAMServiceV3(const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT;
    
    [[nodiscard]] IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT;
    [[nodiscard]] BlocksRAMRequestId open_request() SKR_NOEXCEPT;
    RAMIOBufferId request(IORequestId request, skr_io_future_t* future, SkrAsyncServicePriority priority) SKR_NOEXCEPT;
    void request(IOBatchId request) SKR_NOEXCEPT;
    
    void cancel(skr_io_future_t* future) SKR_NOEXCEPT 
    { 
        skr_atomicu32_store_relaxed(&future->request_cancel, 1); 
    }

    virtual void tick(ESkrIOTickStage type) SKR_NOEXCEPT;
    virtual uint64_t remaining_count(ESkrIOTickStage type) SKR_NOEXCEPT;

    const skr::String name;
    SmartPoolPtr<RAMRequestMixin, IBlocksRAMRequest> request_pool = nullptr;
    SmartPoolPtr<RAMIOBuffer, IRAMIOBuffer> ram_buffer_pool = nullptr;
    SmartPoolPtr<RAMIOBatch, IIOBatch> ram_batch_pool = nullptr;
    
private:
    RAMIOBuffer allocateBuffer(uint64_t n) SKR_NOEXCEPT;
    void freeBuffer(RAMIOBuffer* buffer) SKR_NOEXCEPT;

    static uint32_t global_idx;
    SAtomicU64 request_sequence = 0;
    SAtomicU64 batch_sequence = 0;

    SAtomicU64 tick_counts[SKR_IO_TICK_STAGE_COUNT];
    IOBatchQueue loading_batches[SKR_ASYNC_SERVICE_PRIORITY_COUNT];

    skr::Vector<RAMServiceAction> actions;
    struct RAMInitAndGoal {
        RAMWorldState init;
        RAMWorldState goal;

        bool operator==(const RAMInitAndGoal& rhs) const SKR_NOEXCEPT 
        { 
            return (init == rhs.init) && (goal == rhs.goal);
        }
        
        struct Hash {
            size_t operator()(const RAMInitAndGoal& rhs) const SKR_NOEXCEPT 
            { 
                return skr_hash64(&rhs, sizeof(RAMInitAndGoal), 0); 
            }
        };
    };
    container::LRU::Cache<RAMInitAndGoal, SPtr<PlanType>, RAMInitAndGoal::Hash> plans;
};

} // namespace io
} // namespace skr