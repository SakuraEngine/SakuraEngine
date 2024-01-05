#include "SkrRT/misc/log.h"
#include "SkrRT/platform/crash.h"
#include "SkrRT/goap/planner.hpp"
#include "SkrTestFramework/framework.hpp"

#include <iostream>
#include <vector>

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
    GoapTests()
    {
    }
    ~GoapTests()
    {
    }
};

TEST_CASE_METHOD(GoapTests, "I/O")
{
    using WorldState = skr::goap::WorldState<int, bool>;
    using Action     = skr::goap::Action<WorldState>;
    using Planner    = skr::goap::Planner<WorldState>;

    skr::Vector<Action> actions;
    // states
    static const int file_opened   = 15;
    static const int raw_allocated = 10;
    static const int block_readed  = 20;
    static const int decompressd   = 30;
    static const int raw_freed     = 40;
    static const int file_closed   = 50;

    static const int cancelling = 100;

    // actions
    Action openFile(u8"openFile");
    openFile.add_effect(file_opened, true);
    actions.push_back(openFile);

    Action allocateMemory(u8"allocateMemory");
    allocateMemory.add_condition(cancelling, false);
    allocateMemory.add_condition(file_opened, true);
    allocateMemory.add_effect(raw_allocated, true);
    actions.push_back(allocateMemory);

    Action readBytes(u8"readBytes");
    readBytes.add_condition(cancelling, false);
    readBytes.add_condition(file_opened, true);
    readBytes.add_condition(raw_allocated, true);
    readBytes.add_effect(block_readed, true);
    actions.push_back(readBytes);

    Action decompress(u8"decompress");
    decompress.add_condition(cancelling, false);
    decompress.add_condition(block_readed, true);
    decompress.add_effect(decompressd, true);
    actions.push_back(decompress);

    Action freeRaw(u8"freeRaw");
    freeRaw.add_condition(cancelling, false);
    freeRaw.add_condition(raw_allocated, true);
    freeRaw.add_condition(decompressd, true);
    freeRaw.add_effect(raw_freed, true);
    actions.push_back(freeRaw);

    Action closeFile(u8"closeFile");
    closeFile.add_condition(cancelling, false);
    closeFile.add_condition(block_readed, true);
    closeFile.add_condition(file_opened, true);
    closeFile.add_effect(file_closed, true);
    actions.push_back(closeFile);

    Action freeRaw_Cancel(u8"freeRaw(Cancel)");
    freeRaw_Cancel.add_condition(cancelling, true);
    freeRaw_Cancel.add_condition(raw_allocated, true);
    freeRaw_Cancel.add_effect(raw_freed, true);
    actions.push_back(freeRaw_Cancel);

    Action closeFile_Cancel(u8"closeFile(Cancel)");
    closeFile_Cancel.add_condition(cancelling, true);
    closeFile_Cancel.add_condition(file_opened, true);
    closeFile_Cancel.add_effect(file_closed, true);
    actions.push_back(closeFile_Cancel);

    WorldState initial_state;
    initial_state.set_variable(cancelling, false);
    // NO DECOMPRESS
    {
        WorldState goal;
        goal.set_variable(block_readed, true);
        goal.set_variable(file_closed, true);
        // Fire up the A* planner
        Planner             as;
        skr::Vector<Action> the_plan = as.plan(initial_state, goal, actions);
        std::cout << "NO DECOMPRESS: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
    }
    // WITH DECOMPRESS
    {
        WorldState goal;
        goal.set_variable(decompressd, true);
        goal.set_variable(raw_freed, true);
        goal.set_variable(file_closed, true);
        // Fire up the A* planner
        Planner             as;
        skr::Vector<Action> the_plan = as.plan(initial_state, goal, actions);
        std::cout << "WITH DECOMPRESS: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
    }
    // REQUEST CANCEL
    {
        WorldState current;
        current.set_variable(file_opened, true);
        current.set_variable(raw_allocated, true);
        current.set_variable(block_readed, true);
        current.set_variable(cancelling, true);
        WorldState cancelled;
        cancelled.set_variable(file_closed, true);
        cancelled.set_variable(raw_freed, true);
        cancelled.set_variable(cancelling, true);
        // Fire up the A* planner
        Planner             as;
        skr::Vector<Action> the_plan = as.plan(current, cancelled, actions);
        std::cout << "REQUEST CANCEL: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
    }
}
