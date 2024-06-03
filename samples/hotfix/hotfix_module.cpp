#include "hotfix_module.hpp"
#include "SkrCore/memory/memory.h"
#include "SkrCore/log.h"

namespace hotfix
{
    void HotfixModule::on_load(int argc, char8_t** argv)
    {
        SKR_LOG_INFO(u8"HotfixModule::on_load");
        state = SkrNew<State>();
        get_state()->counter = 0;
    }

    void HotfixModule::on_unload()
    {
        SKR_LOG_INFO(u8"HotfixModule::on_unload");
        SkrDelete(get_state());
    }

    void HotfixModule::on_reload_begin()
    {
        SKR_LOG_INFO(u8"HotfixModule::on_reload_begin");
    }

    void HotfixModule::on_reload_finish()
    {
        SKR_LOG_INFO(u8"HotfixModule::on_reload_finish");
    }

    bool HotfixModule::Tick()
    {
        auto& counter = get_state()->counter;
        counter+=3;
        if(counter > 20)
        {
            counter = 0;
            SKR_LOG_DEBUG(u8"HotfixModule::Tick: reset");
        }
        SKR_LOG_INFO(u8"HotfixModule::Tick: %d", get_state()->counter);
        return true;
    }
}

IMPLEMENT_DYNAMIC_MODULE(hotfix::HotfixModule, HotfixTest);