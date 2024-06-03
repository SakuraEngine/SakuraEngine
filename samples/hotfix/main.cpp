
#include "SkrCore/module/module_manager.hpp"
#include "hotfix_module.hpp"
#include <thread>
#include <chrono>


int main(int argc, char** argv)
{
    skr::ModuleManager* module_manager = skr_get_module_manager();
    module_manager->enable_hotfix_for_module(u8"HotfixTest");
    module_manager->make_module_graph(u8"HotfixTest", true);
    module_manager->init_module_graph(argc, (char8_t**)argv);
    /*while(1)
    {
        hotfix::HotfixModule* hotfix_module = (hotfix::HotfixModule*)module_manager->get_module(u8"HotfixTest");
        if(!hotfix_module->Tick())
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        module_manager->update();
    }*/
    module_manager->destroy_module_graph();
    return 0;
}