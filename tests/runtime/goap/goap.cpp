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
    static const int file_opened   = 10;
    static const int ram_allocated = 20;
    static const int block_readed  = 30;
    static const int decompressd   = 40;

    static const int cancelling = 100;

    // actions
    Action openFile(u8"openFile");
    openFile.add_effect(file_opened, true);
    actions.push_back(openFile);

    Action allocateMemory(u8"allocateMemory");
    allocateMemory.none_or_equal(cancelling, false);
    allocateMemory.exist_and_equal(file_opened, true);
    allocateMemory.add_effect(ram_allocated, true);
    actions.push_back(allocateMemory);

    Action readBytes(u8"readBytes");
    readBytes.none_or_equal(cancelling, false);
    readBytes.exist_and_equal(file_opened, true);
    readBytes.exist_and_equal(ram_allocated, true);
    readBytes.add_effect(block_readed, true);
    actions.push_back(readBytes);

    Action decompress(u8"decompress");
    decompress.none_or_equal(cancelling, false);
    decompress.exist_and_equal(block_readed, true);
    decompress.add_effect(decompressd, true);
    actions.push_back(decompress);

    Action freeRaw(u8"freeRaw");
    freeRaw.none_or_equal(cancelling, false);
    freeRaw.exist_and_equal(ram_allocated, true);
    freeRaw.exist_and_equal(decompressd, true);
    freeRaw.add_effect(ram_allocated, false);
    actions.push_back(freeRaw);

    Action closeFile(u8"closeFile");
    closeFile.none_or_equal(cancelling, false);
    closeFile.exist_and_equal(block_readed, true);
    closeFile.exist_and_equal(file_opened, true);
    closeFile.add_effect(file_opened, false);
    actions.push_back(closeFile);

    Action freeRaw_Cancel(u8"freeRaw(Cancel)");
    freeRaw_Cancel.exist_and_equal(cancelling, true);
    freeRaw_Cancel.exist_and_equal(ram_allocated, true);
    freeRaw_Cancel.add_effect(ram_allocated, false);
    actions.push_back(freeRaw_Cancel);

    Action closeFile_Cancel(u8"closeFile(Cancel)");
    closeFile_Cancel.exist_and_equal(cancelling, true);
    closeFile_Cancel.exist_and_equal(file_opened, true);
    closeFile_Cancel.add_effect(file_opened, false);
    actions.push_back(closeFile_Cancel);

    WorldState initial_state;
    // NO DECOMPRESS
    {
        WorldState goal;
        goal.set_variable(block_readed, true);
        goal.set_variable(file_opened, false);
        goal.set_variable(ram_allocated, false);
        Planner planner;
        auto    the_plan = planner.plan<true>(initial_state, goal, actions);
        for (int64_t fail_index = the_plan.size() - 1; fail_index >= 0; --fail_index)
        {
            std::cout << "NO DECOMPRESS: Found a path!\n";
            for (int64_t i = the_plan.size() - 1; i >= 0; --i)
            {
                const auto& [action, state] = the_plan[i];
                const bool fail             = (i == (fail_index));
                if (fail)
                {
                    std::cout << "cancel triggered, because action failed: " << (const char*)action.name() << std::endl;

                    auto current = state;
                    current.set_variable(cancelling, true);

                    WorldState cancelled = current;
                    cancelled.assign_variable(file_opened, false);
                    cancelled.assign_variable(ram_allocated, false);
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
            std::cout << "PLAN END\n\n";
        }
    }
    // WITH DECOMPRESS
    {
        WorldState goal;
        goal.set_variable(decompressd, true);
        goal.set_variable(ram_allocated, false);
        goal.set_variable(file_opened, false);
        Planner planner;
        auto    the_plan = planner.plan(initial_state, goal, actions);
        std::cout << "WITH DECOMPRESS: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "PLAN END\n\n";
    }
    // REQUEST CANCEL
    {
        WorldState current;
        current.set_variable(file_opened, true);
        current.set_variable(ram_allocated, true);
        current.set_variable(block_readed, true);
        current.set_variable(cancelling, true);
        WorldState cancelled;
        cancelled.set_variable(file_opened, false);
        cancelled.set_variable(ram_allocated, false);
        Planner planner;
        auto    the_plan = planner.plan(current, cancelled, actions);
        std::cout << "REQUEST CANCEL: Found a path!\n";
        for (int64_t i = the_plan.size() - 1; i >= 0; --i)
        {
            std::cout << "    " << (const char*)the_plan[i].name() << std::endl;
        }
        std::cout << "PLAN END\n\n";
    }
}
