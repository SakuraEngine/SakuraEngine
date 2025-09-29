#include "SkrAnim/resources/animation_resource.hpp"
#include "SkrOS/filesystem.hpp"
#include "SkrBase/misc/defer.hpp"
#include "SkrRuntime/misc/cmd_parser.hpp"
#include "SkrCore/log.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrCore/module/module_manager.hpp"
#include "SkrRuntime/resource/resource_system.hpp"
#include <functional>
#include "SkrRuntime/resource/local_resource_registry.hpp"

#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"

#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"

#include "SkrProfile/profile.h"

bool IsAsset(const skr::Path& path)
{
    if (path.extension(true) == u8".meta")
        return true;
    return false;
}

skr::ShaderResourceFactory* shaderResourceFactory = nullptr;
skr::ShaderOptionsFactory* shaderOptionsFactory = nullptr;
skr::MaterialTypeFactory* matTypeFactory = nullptr;
skr::LocalResourceRegistry* registry = nullptr;

// Animation Factory

skr::SkelFactory* skelFactory = nullptr;
skr::AnimFactory* animFactory = nullptr;

void InitializeResourceSystem(skr::SProject& proj)
{
    using namespace skr::literals;
    auto resource_system = skr::GetResourceSystem();
    registry = SkrNew<skr::LocalResourceRegistry>(proj.GetResourceVFS());
    resource_system->Initialize(registry, proj.GetRamService());

    // shader options factory
    {
        skr::ShaderOptionsFactory::Root factoryRoot = {};
        shaderOptionsFactory = skr::ShaderOptionsFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderOptionsFactory);
    }
    // shader resource factory
    {
        skr::ShaderResourceFactory::Root factoryRoot = {};
        factoryRoot.dont_create_shader = true;
        shaderResourceFactory = skr::ShaderResourceFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderResourceFactory);
    }
    // material type factory
    {
        skr::MaterialTypeFactory::Root factoryRoot = {};
        matTypeFactory = skr::MaterialTypeFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matTypeFactory);
    }
    {
        skelFactory = SkrNew<skr::SkelFactory>();
        resource_system->RegisterFactory(skelFactory);
    }
    {
        animFactory = SkrNew<skr::AnimFactory>();
        resource_system->RegisterFactory(animFactory);
    }
}

void DestroyResourceSystem(skr::SProject& proj)
{
    skr::MaterialTypeFactory::Destroy(matTypeFactory);
    skr::ShaderOptionsFactory::Destroy(shaderOptionsFactory);
    skr::ShaderResourceFactory::Destroy(shaderResourceFactory);

    SkrDelete(skelFactory);
    SkrDelete(animFactory);

    skr::GetResourceSystem()->Shutdown();
    SkrDelete(registry);
}

skr::Vector<skr::SProject*> open_projects(int argc, char** argv)
{
    skr::cmd::parser parser(argc, argv);
    parser.add(u8"project", u8"project path", u8"-p", false);
    parser.add(u8"workspace", u8"workspace path", u8"-w", true);
    if (!parser.parse())
    {
        SKR_LOG_ERROR(u8"Failed to parse command line arguments.");
        return {};
    }
    auto workspace = parser.get<skr::String>(u8"workspace");
    auto projectPath = parser.get_optional<skr::String>(u8"project");
    skr::Vector<skr::SProject*> result;
    {
        auto project = SkrNew<skr::SProject>();
        project->SetEnv(u8"workspace", workspace);
        project->OpenProject(projectPath->c_str());
        result.add(project);
    }
    return result;
}

int compile_project(skr::SProject* project)
{
    auto& system = *skr::GetCookSystem();
    InitializeResourceSystem(*project);
    //----- scan project directory
    skr::stl_vector<skr::Path> paths;
    std::function<void(const skr::Path&)> scanAssetDirectory = [&](const skr::Path& dir) {
        for (skr::fs::DirectoryIterator iter(dir); !iter.at_end(); ++iter)
        {
            const auto& entry = *iter;
            if (entry.type == skr::fs::FileType::Directory)
            {
                scanAssetDirectory(entry.path);
            }
            else if (entry.type == skr::fs::FileType::Regular)
            {
                if (IsAsset(entry.path))
                {
                    paths.push_back(entry.path);
                    SKR_LOG_FMT_DEBUG(u8"{}", entry.path.string().c_str());
                }
            }
        }
    };
    // find all .meta files in the project asset directory and push them to paths
    scanAssetDirectory(skr::Path{ project->GetAssetPath() });
    SKR_LOG_INFO(u8"Project dir scan finished.");
    //----- load project asset meta data (guid & type & path)
    {
        using iter_t = typename decltype(paths)::iterator;
        skr::parallel_for(paths.begin(), paths.end(), 20, [&](iter_t begin, iter_t end) {
            SkrZoneScopedN("LoadMeta");
            for (auto i = begin; i != end; ++i)
            {
                // Calculate relative path
                skr::Path assetPath{ project->GetAssetPath() };
                auto relpath = (*i).relative_to(assetPath);
                system.LoadAssetMeta(project, relpath.string());
            }
        });
    }
    SKR_LOG_INFO(u8"Project asset import finished.");
    // Output directories are now managed by the cook system itself
    //----- schedule cook tasks (checking dependencies)
    {
        system.ParallelForEachAsset(1,
            [&](skr::Span<skr::RC<skr::AssetMetaFile>> assets) {
                SkrZoneScopedN("Cook");
                for (auto asset : assets)
                {
                    system.EnsureCooked(asset->GetGUID());
                }
            });
    }
    SKR_LOG_INFO(u8"Project asset import finished.");
    auto resource_system = skr::GetResourceSystem();
    skr::task::schedule([&] {
        system.WaitForAll();
        resource_system->Quit();
    },
        nullptr);
    resource_system->Update();
    //----- wait
    while (!system.AllCompleted() && resource_system->WaitRequest())
    {
        resource_system->Update();
    }
    DestroyResourceSystem(*project);
    return 0;
}

int compile_all(int argc, char** argv)
{
    skr_log_set_level(SKR_LOG_LEVEL_INFO);

    skr::task::scheduler_t scheduler;
    scheduler.initialize(skr::task::scheudler_config_t());
    scheduler.bind();
    auto& system = *skr::GetCookSystem();
    system.Initialize();
    //----- register project
    auto projects = open_projects(argc, argv);
    SKR_DEFER({
        for (auto& project : projects)
        {
            project->CloseProject();
            SkrDelete(project);
        }
    });
    for (auto& project : projects)
        compile_project(project);

    scheduler.unbind();
    system.Shutdown();

    return 0;
}

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    auto root = skr::fs::current_directory();
    {
        FrameMark;
        SkrZoneScopedN("Initialize");
        moduleManager->mount(root.string().c_str());
        moduleManager->make_module_graph(u8"SkrResourceCompiler", true);
        moduleManager->init_module_graph(argc, argv);
    }
    {
        FrameMark;
        SkrZoneScopedN("CompileAll");
        compile_all(argc, argv);
    }
    {
        FrameMark;
        SkrZoneScopedN("ThreadExit");
        moduleManager->destroy_module_graph();
    }
    return 0;
}