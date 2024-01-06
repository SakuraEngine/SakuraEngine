#include "SkrRT/goap/planner.hpp"
#include "SkrRT/misc/log.h"
#include "SkrRT/platform/crash.h"
#include "SkrTestFramework/framework.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

static struct ProcInitializer {
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        ::skr_finalize_crash_handler();
    }
} init;

struct GoapTests {
protected:
    GoapTests() {}
    ~GoapTests() {}
};

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

    struct IOStates {
        Atom<EFileStatus, u8"file_status">  file_status;
        Atom<ERamStatus, u8"ram_status"> ram_status;
        BoolAtom<u8"block_readed">          block_readed;
        BoolAtom<u8"decompressd">           decompressd;

        BoolAtom<u8"cancelling"> cancelling;
    };
    using StaticWorldState = skr::goap::StaticWorldState<IOStates, u8"IOStates">;
    using Action           = skr::goap::Action<StaticWorldState>;
    using Planner          = skr::goap::Planner<StaticWorldState>;

    // clang-format off
    skr::Vector<Action> actions;
    actions.emplace(u8"openFile").ref()
        .none_or_equal<&IOStates::cancelling>(false)
        .none_or_equal<&IOStates::file_status>(EFileStatus::Closed)
        .add_effect<&IOStates::ram_status>(ERamStatus::SizeAcquired)
        .add_effect<&IOStates::file_status>(EFileStatus::Opened);

    actions.emplace(u8"allocateMemory").ref()
        .none_or_equal<&IOStates::cancelling>(false)
        .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
        .exist_and_equal<&IOStates::ram_status>(ERamStatus::SizeAcquired)
        .add_effect<&IOStates::ram_status>(ERamStatus::Allocated);

    actions.emplace(u8"readBytes").ref()
        .none_or_equal<&IOStates::cancelling>(false)
        .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
        .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
        .add_effect<&IOStates::block_readed>(true);

    actions.emplace(u8"decompress").ref()
        .none_or_equal<&IOStates::cancelling>(false)
        .exist_and_equal<&IOStates::block_readed>(true)
        .add_effect<&IOStates::decompressd>(true);

    actions.emplace(u8"freeRaw").ref()
        .none_or_equal<&IOStates::cancelling>(false)
        .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
        .exist_and_equal<&IOStates::decompressd>(true)
        .add_effect<&IOStates::ram_status>(ERamStatus::Freed);

    actions.emplace(u8"closeFile").ref()
        .none_or_equal<&IOStates::cancelling>(false)
        .exist_and_equal<&IOStates::block_readed>(true)
        .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
        .add_effect<&IOStates::file_status>(EFileStatus::Closed);

    actions.emplace(u8"freeRaw(Cancel)").ref()
        .exist_and_equal<&IOStates::cancelling>(true)
        .exist_and_equal<&IOStates::ram_status>(ERamStatus::Allocated)
        .add_effect<&IOStates::ram_status>(ERamStatus::Freed);

    actions.emplace(u8"closeFile(Cancel)").ref()
        .exist_and_equal<&IOStates::cancelling>(true)
        .exist_and_equal<&IOStates::file_status>(EFileStatus::Opened)
        .add_effect<&IOStates::file_status>(EFileStatus::Closed);
    // clang-format on

    StaticWorldState initial_state;
    // NO DECOMPRESS
    {
        auto goal = StaticWorldState()
                    .set<&IOStates::block_readed>(true)
                    .set<&IOStates::file_status>(EFileStatus::Closed)
                    .set<&IOStates::ram_status>(ERamStatus::Allocated);
        Planner planner;
        auto    the_plan = planner.plan<true>(initial_state, goal, actions);
        for (int64_t fail_index = the_plan.size() - 1; fail_index >= 0; --fail_index)
        {
            std::cout << "[STATIC] NO DECOMPRESS: Found a path!\n";
            for (int64_t i = the_plan.size() - 1; i >= 0; --i)
            {
                const auto& [action, state] = the_plan[i];
                const bool fail             = (i == (fail_index));
                if (fail)
                {
                    std::cout << "cancel triggered, because action failed: "
                              << (const char*)action.name() << std::endl;

                    StaticWorldState current = state;
                    current.set<&IOStates::cancelling>(true);

                    StaticWorldState cancelled = current;
                    cancelled.assign<&IOStates::file_status>(EFileStatus::Closed)
                            .assign<&IOStates::ram_status>(ERamStatus::Freed);
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
        auto goal = StaticWorldState()
                    .set<&IOStates::decompressd>(true)
                    .set<&IOStates::ram_status>(ERamStatus::Freed)
                    .set<&IOStates::file_status>(EFileStatus::Closed);
        Planner planner;
        auto    the_plan = planner.plan(initial_state, goal, actions);
        std::cout << "[STATIC] WITH DECOMPRESS: Found a path!\n";
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
                       .set<&IOStates::block_readed>(true)
                       .set<&IOStates::cancelling>(true);
        auto cancelled = StaticWorldState()
                         .set<&IOStates::file_status>(EFileStatus::Closed)
                         .set<&IOStates::ram_status>(ERamStatus::Freed);
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
