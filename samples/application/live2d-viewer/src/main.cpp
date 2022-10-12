#include "module/module_manager.hpp"
#include "ghc/filesystem.hpp"
#include "utils/log.h"

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = ghc::filesystem::current_path(ec);
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("Live2DViewer", true);
    auto result = moduleManager->init_module_graph(argc, argv);
    if (result != 0) {
        SKR_LOG_ERROR("module graph init failed!");
    }
    moduleManager->destroy_module_graph();
    return 0;
}