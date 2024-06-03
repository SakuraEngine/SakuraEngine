#include "SkrBase/config.h"
#include "SkrCore/module/module.hpp"
namespace hotfix
{
    struct State
    {
        int64_t counter;
    };

    struct HotfixModule : skr::IHotfixModule
    {
        State* get_state()
        {
            return (State*)state;
        }
        void on_load(int argc, char8_t** argv) override;
        void on_unload() override;
        int main_module_exec(int argc, char8_t** argv) override { return 0; }
        void on_reload_begin() override;
        void on_reload_finish() override;

        virtual bool Tick();
    };
}