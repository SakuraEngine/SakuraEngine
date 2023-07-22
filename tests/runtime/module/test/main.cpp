#include "SkrRT/module/module_manager.hpp"
#include "SkrRT/platform/filesystem.hpp"

#include "SkrTestFramework/framework.hpp"

class ModuleTest
{
public:
    ModuleTest()
    {
    }
    ~ModuleTest()
    {
    }
};

TEST_CASE_METHOD(ModuleTest, "single")
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto path = skr::filesystem::current_path(ec);
    moduleManager->mount(path.u8string().c_str());
    EXPECT_NE(moduleManager->make_module_graph(u8"SkrRT", true), nullptr);
    REQUIRE(moduleManager->init_module_graph(0, (char8_t**)nullptr));
    REQUIRE(moduleManager->destroy_module_graph());
}

// dynamic1 -> static0 -> dynamic0
// static modules must be explicit linked
#include "static0.hpp"
TEST_CASE_METHOD(ModuleTest, "dependency")
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto path = skr::filesystem::current_path(ec);
    moduleManager->mount(path.u8string().c_str());
    EXPECT_NE(moduleManager->make_module_graph(u8"dynamic1", true), nullptr);
    REQUIRE(moduleManager->init_module_graph(0, (char8_t**)nullptr));
    REQUIRE(moduleManager->destroy_module_graph());
}

// dynamic3 -> dynamic2 -> dynamic1 ..
TEST_CASE_METHOD(ModuleTest, "dynamic_patch")
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto path = skr::filesystem::current_path(ec);
    moduleManager->mount(path.u8string().c_str());
    EXPECT_NE(moduleManager->make_module_graph(u8"dynamic1", true), nullptr);
    REQUIRE(moduleManager->init_module_graph(0, (char8_t**)nullptr));
    SKR_LOG_INFO(u8"----begins dynamic patch----");
    REQUIRE(moduleManager->patch_module_graph(u8"dynamic3"));
    SKR_LOG_INFO(u8"----ends dynamic patch----");
    REQUIRE(moduleManager->destroy_module_graph());
}