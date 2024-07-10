#include "SkrRT/goap/planner.hpp"
#include "SkrCore/log.h"
#include "SkrCore/crash.h"
#include "SkrTestFramework/framework.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

static struct ProcInitializer {
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        // ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        // ::skr_finalize_crash_handler();
    }
} init;

struct GoapTests {
protected:
    GoapTests() {}
    ~GoapTests() {}
};

TEST_CASE_METHOD(GoapTests, "BatchSync")
{
    using namespace skr;
    using namespace skr::goap;

    enum class EBatchStatus
    {
        Combined,
        Split
    };
    enum class EActionStatus
    {
        Opened, // batch & open
        Seeked, // split & seek
        Readed, // batch & read
        UnZipped, // split & unzip
        Outputed // batch & output
    };
    enum class EPreferFlag
    {
        PreferSplitOpen = 0x0000'0001,
        PreferDStorage = 0x0000'0002,
    };
    using PreferFlags = uint32_t;
    struct IOStates {
        Atom<EBatchStatus, u8"batch_status"> batch_status;
        Atom<EActionStatus, u8"action_status"> action_status;
        Atom<PreferFlags, u8"prefer_flags"> prefer_flags;
    };
    using StaticWorldState = skr::goap::StaticWorldState<IOStates, u8"IOStates">;
    struct Action : public skr::goap::Action<StaticWorldState>
    {
        Action(const char8_t* name, CostType cost = 0) SKR_NOEXCEPT
            : skr::goap::Action<StaticWorldState>(name, cost)
        {
        }
        Action& as_split_action()
        {
            add_effect<&IOStates::batch_status>(EBatchStatus::Split);
            return *this;
        }
        Action& as_batch_action()
        {
            exist_and_equal<&IOStates::batch_status>(EBatchStatus::Combined);
            return *this;
        }
    };
    using Planner          = skr::goap::Planner<StaticWorldState, Action>;

    skr::Vector<Action> actions;
    actions.emplace(u8"sync_combine").ref()
        .exist_and_equal<&IOStates::batch_status>(EBatchStatus::Split)
        .add_effect<&IOStates::batch_status>(EBatchStatus::Combined);

    actions.emplace(u8"open(batch)").ref()
        .as_batch_action()
        .without_flag<&IOStates::prefer_flags>(EPreferFlag::PreferSplitOpen)
        .add_effect<&IOStates::action_status>(EActionStatus::Opened);

    actions.emplace(u8"open(split)").ref()
        .as_split_action()
        .with_flag<&IOStates::prefer_flags>(EPreferFlag::PreferSplitOpen)
        .add_effect<&IOStates::action_status>(EActionStatus::Opened);

    actions.emplace(u8"seek").ref()
        .as_split_action()
        .exist_and_equal<&IOStates::action_status>(EActionStatus::Opened)
        .add_effect<&IOStates::action_status>(EActionStatus::Seeked);

    actions.emplace(u8"read").ref()
        .as_batch_action()
        .exist_and_equal<&IOStates::action_status>(EActionStatus::Seeked)
        .add_effect<&IOStates::action_status>(EActionStatus::Readed);

    actions.emplace(u8"unzip").ref()
        .as_split_action()
        .exist_and_equal<&IOStates::action_status>(EActionStatus::Readed)
        .add_effect<&IOStates::action_status>(EActionStatus::UnZipped);

    actions.emplace(u8"output").ref()
        .as_batch_action()
        .exist_and_equal<&IOStates::action_status>(EActionStatus::UnZipped)
        .add_effect<&IOStates::action_status>(EActionStatus::Outputed);

    {
        auto init = StaticWorldState()
                    .set<&IOStates::prefer_flags>(0)
                    .set<&IOStates::batch_status>(EBatchStatus::Combined);
        auto goal = StaticWorldState()
                    .set<&IOStates::action_status>(EActionStatus::Outputed);
        Planner planner;
        auto    the_plan = planner.plan(init, goal, actions);
        std::cout << "Prefer Batch:\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
    }
    {
        auto init = StaticWorldState()
                    .set<&IOStates::prefer_flags>(EPreferFlag::PreferSplitOpen)
                    .set<&IOStates::batch_status>(EBatchStatus::Combined);
        auto goal = StaticWorldState()
                    .set<&IOStates::action_status>(EActionStatus::Outputed);
        Planner planner;
        auto    the_plan = planner.plan(init, goal, actions);
        std::cout << "Prefer Split:\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
    }

}

TEST_CASE_METHOD(GoapTests, "I/O Static")
{
    using namespace skr;
    using namespace skr::goap;

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
        Reading,
        Readed
    };

    enum class EDecompressStatus
    {
        Hardware,
        Allocated,
        Decompressing,
        Decompressed,
    };

    enum class ECancelStatus
    {
        Cancelling,
        Cancelled
    };

    enum class EChunkStrategy
    {
        SMALL_LOW_LATENCY,
        HUGE_HIGH_THROUGHPUT
    };

    enum class EBatchStrategy
    {
        NVMEBypass,
        DirectMemoryMap
    };

    struct IOStates {
        Atom<EFileStatus, u8"file_status">       file_status;
        Atom<ERamStatus, u8"ram_status">         ram_status;
        Atom<EDecompressStatus, u8"decompressd"> decompress_status;
        Atom<EBlockStatus, u8"block_readed">     block_status;
        Atom<EChunkStrategy, u8"chunk_stragegy"> chunk_stragegy;
        Atom<EBatchStrategy, u8"batch_stragegy"> batch_stragegy;
        Atom<ECancelStatus, u8"cancelling">      cancel_status;
    };
    using StaticWorldState = skr::goap::StaticWorldState<IOStates, u8"IOStates">;
    using Action           = skr::goap::Action<StaticWorldState>;
    using Planner          = skr::goap::Planner<StaticWorldState>;

    // clang-format off
    skr::Vector<Action> actions;
    actions.emplace(u8"openFile").ref()
        .none<&IOStates::cancel_status>()
        .none_or_equal<&IOStates::file_status>(EFileStatus::Closed)
        .add_effect<&IOStates::ram_status>(ERamStatus::SizeAcquired)
        .add_effect<&IOStates::file_status>(EFileStatus::Opened);

    { // memory allocates
        actions.emplace(u8"allocateMemory").ref()
            .none<&IOStates::cancel_status>()
            .none_or_nequal<&IOStates::batch_stragegy>(EBatchStrategy::DirectMemoryMap)
            .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
            .exist_and_equal<&IOStates::ram_status>(ERamStatus::SizeAcquired)
            .add_effect<&IOStates::ram_status>(ERamStatus::Allocated);

        actions.emplace(u8"allocateMemory(Chunk)").ref()
            .none<&IOStates::cancel_status>()
            .none_or_nequal<&IOStates::batch_stragegy>(EBatchStrategy::DirectMemoryMap)
            .exist<&IOStates::chunk_stragegy>()
            .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
            .exist_and_equal<&IOStates::ram_status>(ERamStatus::SizeAcquired)
            .add_effect<&IOStates::ram_status>(ERamStatus::Allocated);

        actions.emplace(u8"checkMemoryMapping(Hardware)").ref()
            .none<&IOStates::cancel_status>()
            .none<&IOStates::chunk_stragegy>()
            .exist_and_equal<&IOStates::batch_stragegy>(EBatchStrategy::DirectMemoryMap)
            .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
            .exist_and_equal<&IOStates::ram_status>(ERamStatus::SizeAcquired)
            .add_effect<&IOStates::ram_status>(ERamStatus::Allocated);
    }
    {
        actions.emplace(u8"prepare_softwareDecompress").ref()
            .none<&IOStates::cancel_status>()
            .exist_and_equal<&IOStates::ram_status>(ERamStatus::SizeAcquired)
            .none_or_nequal<&IOStates::decompress_status>(EDecompressStatus::Hardware)
            .add_effect<&IOStates::decompress_status>(EDecompressStatus::Allocated);
    }
    { // read plain blocks & chunks with OS-Sync file API
        actions.emplace(u8"schedule_readBytes").ref()
            .none<&IOStates::cancel_status>()
            .none<&IOStates::batch_stragegy>()
            .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
            .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
            .add_effect<&IOStates::block_status>(EBlockStatus::Reading);
        actions.emplace(u8"acquire_readBytes").ref()
            .none<&IOStates::cancel_status>()
            .none<&IOStates::batch_stragegy>()
            .exist_and_equal<&IOStates::block_status>(EBlockStatus::Reading)
            .add_effect<&IOStates::block_status>(EBlockStatus::Readed);
    }
    { // read batch blocks with NVME or Console bypass API
        actions.emplace(u8"emit_readRequest").ref()
            .none<&IOStates::cancel_status>()
            .exist<&IOStates::batch_stragegy>()
            .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
            .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
            .add_effect<&IOStates::block_status>(EBlockStatus::Reading);
        actions.emplace(u8"acq_readRequest").ref()
            .none<&IOStates::cancel_status>()
            .exist<&IOStates::batch_stragegy>()
            .exist_and_equal<&IOStates::block_status>(EBlockStatus::Reading)
            .add_effect<&IOStates::block_status>(EBlockStatus::Readed);
    }
    {
        actions.emplace(u8"schedule_softwareDecompress").ref()
            .none<&IOStates::cancel_status>()
            .exist_and_equal<&IOStates::block_status>(EBlockStatus::Readed)
            .exist_and_equal<&IOStates::decompress_status>(EDecompressStatus::Allocated)
            .add_effect<&IOStates::decompress_status>(EDecompressStatus::Decompressing);

        actions.emplace(u8"acquire_softwareDecompress").ref()
            .none<&IOStates::cancel_status>()
            .exist_and_equal<&IOStates::decompress_status>(EDecompressStatus::Decompressing)
            .add_effect<&IOStates::decompress_status>(EDecompressStatus::Decompressed);

        actions.emplace(u8"acquire_hardwareDecompress").ref()
            .none<&IOStates::cancel_status>()
            .exist_and_equal<&IOStates::decompress_status>(EDecompressStatus::Hardware)
            .exist_and_equal<&IOStates::block_status>(EBlockStatus::Readed)
            .add_effect<&IOStates::decompress_status>(EDecompressStatus::Decompressed);
    }

    actions.emplace(u8"freeRaw").ref()
        .none<&IOStates::cancel_status>()
        .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
        .exist_and_equal<&IOStates::decompress_status>(EDecompressStatus::Decompressed)
        .add_effect<&IOStates::ram_status>(ERamStatus::Freed);

    actions.emplace(u8"closeFile").ref()
        .none<&IOStates::cancel_status>()
        .exist_and_equal<&IOStates::block_status>(EBlockStatus::Readed)
        .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
        .add_effect<&IOStates::file_status>(EFileStatus::Closed);

    actions.emplace(u8"freeRaw(Cancel)").ref()
        .exist_and_equal<&IOStates::cancel_status>(ECancelStatus::Cancelling)
        .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
        .add_effect<&IOStates::ram_status>(ERamStatus::Freed);

    actions.emplace(u8"closeFile(Cancel)").ref()
        .exist_and_equal<&IOStates::cancel_status>(ECancelStatus::Cancelling)
        .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
        .add_effect<&IOStates::file_status>(EFileStatus::Closed);

    actions.emplace(u8"acquireSuccess(Cancel)").ref()
        .exist_and_equal<&IOStates::cancel_status>(ECancelStatus::Cancelling)
        .none_or_nequal<&IOStates::ram_status>(ERamStatus::Allocated)
        .none_or_nequal<&IOStates::file_status>(EFileStatus::Opened)
        .add_effect<&IOStates::cancel_status>(ECancelStatus::Cancelled);
    // clang-format on

    // NO DECOMPRESS
    {
        auto init = StaticWorldState();
        auto goal = StaticWorldState()
                    .set<&IOStates::block_status>(EBlockStatus::Readed)
                    .set<&IOStates::file_status>(EFileStatus::Closed)
                    .set<&IOStates::ram_status>(ERamStatus::Allocated);
        Planner planner;
        auto    the_plan = planner.plan<true>(init, goal, actions);
        for (int64_t cancel_index = the_plan.size() - 1; cancel_index >= 0; --cancel_index)
        {
            std::cout << "[STATIC] NO DECOMPRESS: Found a path!\n";
            for (int64_t i = the_plan.size() - 1; i >= 0; --i)
            {
                const bool  last        = (i == (the_plan.size() - 1));
                const auto& action      = the_plan[i].first;
                const auto& state       = !last ? the_plan[i + 1].second : init;
                const bool  mock_cancel = (i == (cancel_index));
                if (mock_cancel)
                {
                    std::cout << "cancel triggered, before action: "
                              << (const char*)action.name() << std::endl;

                    StaticWorldState current = state;
                    current.set<&IOStates::cancel_status>(ECancelStatus::Cancelling);

                    auto cancelled = StaticWorldState()
                                     .set<&IOStates::cancel_status>(ECancelStatus::Cancelled);
                    Planner cancel_planner;
                    auto    cancel_plan = cancel_planner.plan(current, cancelled, actions);
                    std::cout << "    REQUEST CANCEL:\n";
                    for (int64_t i = cancel_plan.size() - 1; i >= 0; --i)
                    {
                        std::cout << "        " << (const char*)cancel_plan[i].name() << std::endl;
                    }
                    break;
                }
                else
                    std::cout << "    " << (const char*)action.name() << std::endl;
            }
            std::cout << "[STATIC] PLAN END\n\n";
        }
    }
    // WITH DECOMPRESS
    {
        auto init = StaticWorldState();
        auto goal = StaticWorldState()
                    .set<&IOStates::decompress_status>(EDecompressStatus::Decompressed)
                    .set<&IOStates::ram_status>(ERamStatus::Freed)
                    .set<&IOStates::file_status>(EFileStatus::Closed);
        Planner planner;
        auto    the_plan = planner.plan(init, goal, actions);
        std::cout << "[STATIC] WITH DECOMPRESS: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "[STATIC] PLAN END\n\n";
    }
    // WITH HARDWARE DECOMPRESS
    {
        auto init = StaticWorldState()
                    .set<&IOStates::decompress_status>(EDecompressStatus::Hardware);
        auto goal = StaticWorldState()
                    .set<&IOStates::decompress_status>(EDecompressStatus::Decompressed)
                    .set<&IOStates::ram_status>(ERamStatus::Freed)
                    .set<&IOStates::file_status>(EFileStatus::Closed);
        Planner planner;
        auto    the_plan = planner.plan(init, goal, actions);
        std::cout << "[STATIC] WITH HARDWARE DECOMPRESS: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "[STATIC] PLAN END\n\n";
    }
    // REQUEST CANCEL
    {
        auto current = StaticWorldState()
                       .set<&IOStates::file_status>(EFileStatus::Opened)
                       .set<&IOStates::ram_status>(ERamStatus::Allocated)
                       .set<&IOStates::block_status>(EBlockStatus::Readed)
                       .set<&IOStates::cancel_status>(ECancelStatus::Cancelling);
        auto cancelled = StaticWorldState()
                         .assign<&IOStates::cancel_status>(ECancelStatus::Cancelled);
        Planner planner;
        auto    the_plan = planner.plan(current, cancelled, actions);
        std::cout << "[STATIC] REQUEST CANCEL: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "[STATIC] PLAN END\n\n";
    }
    // clang-format on
}

TEST_CASE_METHOD(GoapTests, "I/O Dynamic")
{
    using DynamicWorldState = skr::goap::DynamicWorldState<int, bool>;
    using Action            = skr::goap::Action<DynamicWorldState>;
    using Planner           = skr::goap::Planner<DynamicWorldState>;

    // states
    static const int file_opened   = 10;
    static const int ram_allocated = 20;
    static const int block_readed  = 30;
    static const int decompressd   = 40;

    static const int cancelling = 100;

    // clang-format off
    skr::Vector<Action> actions;
    actions.emplace(u8"openFile").ref()
        .none_or_equal(cancelling, false)
        .add_effect(file_opened, true);

    actions.emplace(u8"allocateMemory").ref()
        .none_or_equal(cancelling, false)
        .exist_and_equal(file_opened, true)
        .add_effect(ram_allocated, true);

    actions.emplace(u8"readBytes").ref()
        .none_or_equal(cancelling, false)
        .exist_and_equal(file_opened, true)
        .exist_and_equal(ram_allocated, true)
        .add_effect(block_readed, true);

    actions.emplace(u8"decompress").ref()
        .none_or_equal(cancelling, false)
        .exist_and_equal(block_readed, true)
        .add_effect(decompressd, true);

    actions.emplace(u8"freeRaw").ref()
        .none_or_equal(cancelling, false)
        .exist_and_equal(ram_allocated, true)
        .exist_and_equal(decompressd, true)
        .add_effect(ram_allocated, false);

    actions.emplace(u8"closeFile").ref()
        .none_or_equal(cancelling, false)
        .exist_and_equal(block_readed, true)
        .exist_and_equal(file_opened, true)
        .add_effect(file_opened, false);

    actions.emplace(u8"freeRaw(Cancel)").ref()
        .exist_and_equal(cancelling, true)
        .exist_and_equal(ram_allocated, true)
        .add_effect(ram_allocated, false);

    actions.emplace(u8"closeFile(Cancel)").ref()
        .exist_and_equal(cancelling, true)
        .exist_and_equal(file_opened, true)
        .add_effect(file_opened, false);
    // clang-format on

    DynamicWorldState initial_state;
    // NO DECOMPRESS
    {
        auto goal = DynamicWorldState()
                    .set(block_readed, true)
                    .set(file_opened, false)
                    .set(ram_allocated, false);
        Planner planner;
        auto    the_plan = planner.plan<true>(initial_state, goal, actions);
        for (int64_t fail_index = the_plan.size() - 1; fail_index >= 0; --fail_index)
        {
            std::cout << "[DYNAMIC] NO DECOMPRESS: Found a path!\n";
            for (int64_t i = the_plan.size() - 1; i >= 0; --i)
            {
                const auto& [action, state] = the_plan[i];
                const bool fail             = (i == (fail_index));
                if (fail)
                {
                    std::cout << "cancel triggered, because action failed: "
                              << (const char*)action.name() << std::endl;

                    DynamicWorldState current = state;
                    current.set(cancelling, true);

                    DynamicWorldState cancelled = current;
                    cancelled.assign(file_opened, false)
                    .assign(ram_allocated, false);
                    Planner cancel_planner;
                    auto    cancel_plan = cancel_planner.plan(current, cancelled, actions);
                    std::cout << "    REQUEST CANCEL:\n";
                    for (int64_t i = cancel_plan.size() - 1; i >= 0; --i)
                    {
                        std::cout << "        " << (const char*)cancel_plan[i].name() << std::endl;
                    }
                    break;
                }
                else
                    std::cout << "    " << (const char*)action.name() << std::endl;
            }
            std::cout << "[DYNAMIC] PLAN END\n\n";
        }
    }
    // WITH DECOMPRESS
    {
        auto goal = DynamicWorldState()
                    .set(decompressd, true)
                    .set(ram_allocated, false)
                    .set(file_opened, false);
        Planner planner;
        auto    the_plan = planner.plan(initial_state, goal, actions);
        std::cout << "[DYNAMIC] WITH DECOMPRESS: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "[DYNAMIC] PLAN END\n\n";
    }
    // REQUEST CANCEL
    {
        auto current = DynamicWorldState()
                       .set(file_opened, true)
                       .set(ram_allocated, true)
                       .set(block_readed, true)
                       .set(cancelling, true);
        auto cancelled = DynamicWorldState()
                         .set(file_opened, false)
                         .set(ram_allocated, false);
        Planner planner;
        auto    the_plan = planner.plan(current, cancelled, actions);
        std::cout << "[DYNAMIC] REQUEST CANCEL: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "[DYNAMIC] PLAN END\n\n";
    }
}

TEST_CASE_METHOD(GoapTests, "AssetCook")
{
    enum class EAssetPipelineStage
    {
        None,
        Import,
        Cook,
        Save,
        Done
    };
    enum class EAssetPipelineError
    {
        None,
        ImportError,
        CookError,
        SaveError
    };
    struct AssetPipelineAtoms
    {
        skr::goap::Atom<EAssetPipelineStage, u8"AssetPipelineStage"> stage;
        skr::goap::Atom<EAssetPipelineError, u8"AssetPipelineError"> error;
        skr::goap::Atom<bool, u8"ErrorHandled"> handled;
    };
    using AssetPipelineState = skr::goap::StaticWorldState<AssetPipelineAtoms, u8"AssetCookPipeline">;
    
    struct PipelineAction : public skr::goap::Action<AssetPipelineState>
    {
        PipelineAction(const char8_t* name, EAssetPipelineStage stage) SKR_NOEXCEPT
            : skr::goap::Action<AssetPipelineState>(name, 0), is_stage(true), stage(stage)
        {
            const auto last_stage = static_cast<EAssetPipelineStage>(static_cast<uint32_t>(stage) - 1);
            exist_and_equal<&AssetPipelineAtoms::stage>(last_stage);
            exist_and_equal<&AssetPipelineAtoms::error>(EAssetPipelineError::None);
            add_effect<&AssetPipelineAtoms::stage>(stage);
        }
        PipelineAction(const char8_t* name, EAssetPipelineError error) SKR_NOEXCEPT
            : skr::goap::Action<AssetPipelineState>(name, 0), is_stage(false), error(error)
        {
            exist_and_equal<&AssetPipelineAtoms::error>(error);
            add_effect<&AssetPipelineAtoms::handled>(true);
        }
        const bool is_stage = false;
        EAssetPipelineStage get_stage() const { SKR_ASSERT(is_stage); return stage; }
        EAssetPipelineError get_error() const { SKR_ASSERT(!is_stage); return error; }
    private:
        EAssetPipelineStage stage;
        EAssetPipelineError error;
    };
    using PipelinePlanner = skr::goap::Planner<AssetPipelineState, PipelineAction>;

    skr::Vector<PipelineAction> actions;
    actions.emplace(u8"Import", EAssetPipelineStage::Import);
    actions.emplace(u8"Cook", EAssetPipelineStage::Cook);
    actions.emplace(u8"Save", EAssetPipelineStage::Save);
    actions.emplace(u8"Done", EAssetPipelineStage::Done);
    actions.emplace(u8"HandleImportError", EAssetPipelineError::ImportError);
    actions.emplace(u8"HandleCookError", EAssetPipelineError::CookError);
    actions.emplace(u8"HandleSaveError", EAssetPipelineError::SaveError);

    // NORMAL COOK
    {
        auto init = AssetPipelineState();
        init.set<&AssetPipelineAtoms::stage>(EAssetPipelineStage::None);
        init.set<&AssetPipelineAtoms::error>(EAssetPipelineError::None);
        auto goal = AssetPipelineState();
        goal.set<&AssetPipelineAtoms::stage>(EAssetPipelineStage::Done);
        goal.set<&AssetPipelineAtoms::error>(EAssetPipelineError::None);
        PipelinePlanner planner;
        auto the_plan = planner.plan<true>(init, goal, actions);
        uint32_t stage_cursor = 1;
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            auto& [action, state] = the_plan[i];
            EXPECT_TRUE(action.is_stage);
            EXPECT_EQ(action.get_stage(), static_cast<EAssetPipelineStage>(stage_cursor++));
        }
    }

}