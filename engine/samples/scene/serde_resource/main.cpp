#include "SkrCore/module/module_manager.hpp"
#include <SkrOS/filesystem.hpp>

int main(int argc, char* argv[])
{
    auto moduleManager = skr_get_module_manager();
    auto root = skr::fs::current_directory();
    {
        FrameMark;
        SkrZoneScopedN("SceneSerdeResource");
        moduleManager->mount(root.string().c_str());
        moduleManager->make_module_graph(u8"SceneSample_SerdeResource", true);
        moduleManager->init_module_graph(argc, argv);
        moduleManager->destroy_module_graph();
    }
    return 0;
}