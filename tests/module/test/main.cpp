#include "gtest/gtest.h"
#include "module/module_manager.hpp"
#include "ghc/filesystem.hpp"

class ModuleTest : public ::testing::Test
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_F(ModuleTest, single)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto path = ghc::filesystem::current_path(ec);
    moduleManager->mount(path.u8string().c_str());
    EXPECT_NE(moduleManager->make_module_graph("SkrRT", true), nullptr);
    EXPECT_TRUE(moduleManager->init_module_graph());
    EXPECT_TRUE(moduleManager->destroy_module_graph());
}

// dynamic1 -> static0 -> dynamic0
// static modules must be explicit linked
#include "static0.hpp"
TEST_F(ModuleTest, dependency)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto path = ghc::filesystem::current_path(ec);
    moduleManager->mount(path.u8string().c_str());
    EXPECT_NE(moduleManager->make_module_graph("dynamic1", true), nullptr);
    EXPECT_TRUE(moduleManager->init_module_graph());
    EXPECT_TRUE(moduleManager->destroy_module_graph());
}

// dynamic3 -> dynamic2 -> dynamic1 ..
TEST_F(ModuleTest, dynamic_patch)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto path = ghc::filesystem::current_path(ec);
    moduleManager->mount(path.u8string().c_str());
    EXPECT_NE(moduleManager->make_module_graph("dynamic1", true), nullptr);
    EXPECT_TRUE(moduleManager->init_module_graph());
    SKR_LOG_INFO("----begins dynamic patch----");
    EXPECT_TRUE(moduleManager->patch_module_graph("dynamic3"));
    SKR_LOG_INFO("----ends dynamic patch----");
    EXPECT_TRUE(moduleManager->destroy_module_graph());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}