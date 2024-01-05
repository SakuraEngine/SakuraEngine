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
    const int file_opened     = 15;
    const int memory_allocated = 10;
    const int raw_loaded      = 20;
    const int decompressd     = 30;
    const int file_closed     = 40;

    // actions
    Action openFile(u8"openFile");
    openFile.add_effect(file_opened, true);
    actions.push_back(openFile);

    Action allocateMemory(u8"allocateMemory");
    allocateMemory.add_condition(file_opened, true);
    allocateMemory.add_effect(memory_allocated, true);
    actions.push_back(allocateMemory);

    Action readBytes(u8"readBytes");
    readBytes.add_condition(file_opened, true);
    readBytes.add_condition(memory_allocated, true);
    readBytes.add_effect(raw_loaded, true);
    actions.push_back(readBytes);

    Action decompress(u8"decompress");
    decompress.add_condition(raw_loaded, true);
    decompress.add_effect(decompressd, true);
    actions.push_back(decompress);
    
    Action freeRaw(u8"freeRaw");
    freeRaw.add_condition(raw_loaded, true);
    freeRaw.add_condition(decompressd, true);
    freeRaw.add_effect(raw_loaded, false);
    actions.push_back(freeRaw);

    Action closeFile(u8"closeFile");
    closeFile.add_condition(raw_loaded, true);
    closeFile.add_condition(file_opened, true);
    closeFile.add_effect(file_closed, true);
    actions.push_back(closeFile);

    WorldState initial_state;
    // NO DECOMPRESS
    {
        WorldState goal;
        goal.set_variable(raw_loaded, true);
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
        goal.set_variable(raw_loaded, false);
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
}
