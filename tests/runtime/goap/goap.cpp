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

    struct IOStates {
        BoolAtom<u8"file_opened"> file_opened;
        BoolAtom<u8"ram_allocated"> ram_allocated;
        BoolAtom<u8"block_readed"> block_readed;
        BoolAtom<u8"decompressd"> decompressd;

        BoolAtom<u8"cancelling"> cancelling;
    };
    using StaticWorldState = skr::goap::StaticWorldState<IOStates, u8"IOStates">;
    using Action            = skr::goap::Action<StaticWorldState>;
    using Planner           = skr::goap::Planner<StaticWorldState>;
        
    // clang-format off
    skr::Vector<Action> actions;
    actions.emplace(u8"openFile").ref()
        .none_or_equal(atom_id<&IOStates::cancelling>, false)
        .add_effect(atom_id<&IOStates::file_opened>, true);

    actions.emplace(u8"allocateMemory").ref()
        .none_or_equal(atom_id<&IOStates::cancelling>, false)
        .exist_and_equal(atom_id<&IOStates::file_opened>, true)
        .add_effect(atom_id<&IOStates::ram_allocated>, true);

    actions.emplace(u8"readBytes").ref()
        .none_or_equal(atom_id<&IOStates::cancelling>, false)
        .exist_and_equal(atom_id<&IOStates::file_opened>, true)
        .exist_and_equal(atom_id<&IOStates::ram_allocated>, true)
        .add_effect(atom_id<&IOStates::block_readed>, true);

    actions.emplace(u8"decompress").ref()
        .none_or_equal(atom_id<&IOStates::cancelling>, false)
        .exist_and_equal(atom_id<&IOStates::block_readed>, true)
        .add_effect(atom_id<&IOStates::decompressd>, true);

    actions.emplace(u8"freeRaw").ref()
        .none_or_equal(atom_id<&IOStates::cancelling>, false)
        .exist_and_equal(atom_id<&IOStates::ram_allocated>, true)
        .exist_and_equal(atom_id<&IOStates::decompressd>, true)
        .add_effect(atom_id<&IOStates::ram_allocated>, false);

    actions.emplace(u8"closeFile").ref()
        .none_or_equal(atom_id<&IOStates::cancelling>, false)
        .exist_and_equal(atom_id<&IOStates::block_readed>, true)
        .exist_and_equal(atom_id<&IOStates::file_opened>, true)
        .add_effect(atom_id<&IOStates::file_opened>, false);

    actions.emplace(u8"freeRaw(Cancel)").ref()
        .exist_and_equal(atom_id<&IOStates::cancelling>, true)
        .exist_and_equal(atom_id<&IOStates::ram_allocated>, true)
        .add_effect(atom_id<&IOStates::ram_allocated>, false);

    actions.emplace(u8"closeFile(Cancel)").ref()
        .exist_and_equal(atom_id<&IOStates::cancelling>, true)
        .exist_and_equal(atom_id<&IOStates::file_opened>, true)
        .add_effect(atom_id<&IOStates::file_opened>, false);
    // clang-format on

    StaticWorldState initial_state;
    // NO DECOMPRESS
    {
        auto goal = StaticWorldState()
                    .set_variable(atom_id<&IOStates::block_readed>, true)
                    .set_variable(atom_id<&IOStates::file_opened>, false)
                    .set_variable(atom_id<&IOStates::ram_allocated>, false);
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
                    current.set_variable(atom_id<&IOStates::cancelling>, true);

                    StaticWorldState cancelled = current;
                    cancelled.assign_variable(atom_id<&IOStates::file_opened>, false)
                    .assign_variable(atom_id<&IOStates::ram_allocated>, false);
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
                    .set_variable(atom_id<&IOStates::decompressd>, true)
                    .set_variable(atom_id<&IOStates::ram_allocated>, false)
                    .set_variable(atom_id<&IOStates::file_opened>, false);
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
                       .set_variable(atom_id<&IOStates::file_opened>, true)
                       .set_variable(atom_id<&IOStates::ram_allocated>, true)
                       .set_variable(atom_id<&IOStates::block_readed>, true)
                       .set_variable(atom_id<&IOStates::cancelling>, true);
        auto cancelled = StaticWorldState()
                         .set_variable(atom_id<&IOStates::file_opened>, false)
                         .set_variable(atom_id<&IOStates::ram_allocated>, false);
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
                    .set_variable(block_readed, true)
                    .set_variable(file_opened, false)
                    .set_variable(ram_allocated, false);
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
                    current.set_variable(cancelling, true);

                    DynamicWorldState cancelled = current;
                    cancelled.assign_variable(file_opened, false)
                    .assign_variable(ram_allocated, false);
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
                    .set_variable(decompressd, true)
                    .set_variable(ram_allocated, false)
                    .set_variable(file_opened, false);
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
                       .set_variable(file_opened, true)
                       .set_variable(ram_allocated, true)
                       .set_variable(block_readed, true)
                       .set_variable(cancelling, true);
        auto cancelled = DynamicWorldState()
                         .set_variable(file_opened, false)
                         .set_variable(ram_allocated, false);
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
