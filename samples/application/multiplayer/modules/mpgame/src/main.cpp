#include "module/module_manager.hpp"
#include "MPGame/mp_interface.h"
#include "platform/filesystem.hpp"
int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    auto root = skr::filesystem::current_path();
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph(u8"MPGame");
    moduleManager->init_module_graph(argc, argv);
    auto app = CreateMPApplication();
    auto result = app->Initialize();
    if(result != 0)
    {
        app->Shutdown();
        return result;
    }
    app->Run();
    app->Shutdown();
    return 0;
}