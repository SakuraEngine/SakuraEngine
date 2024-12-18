#include "SkrCore/platform/vfs.h"
#include "SkrOS/filesystem.hpp"
#include "SkrBase/misc/defer.hpp"
#include "SkrRT/misc/cmd_parser.hpp"
#include "SkrCore/log.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrContainers/string.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrCore/module/module_manager.hpp"
#include "SkrRT/resource/resource_system.h"
#include "SkrRT/resource/local_resource_registry.hpp"

#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"

#include "SkrAnim/resources/skeleton_resource.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/asset/cook_system.hpp"

#include "SkrProfile/profile.h"

bool IsAsset(skr::filesystem::path path)
{
    if (path.extension() == ".meta")
        return true;
    return false;
}

skr::renderer::SShaderResourceFactory* shaderResourceFactory = nullptr;
skr::renderer::SShaderOptionsFactory* shaderOptionsFactory = nullptr;
skr::renderer::SMaterialTypeFactory* matTypeFactory = nullptr;
skr::resource::SLocalResourceRegistry* registry = nullptr;
skr::resource::SSkelFactory* skelFactory = nullptr;

void InitializeResourceSystem(skd::SProject& proj)
{
    using namespace skr::literals;
    auto resource_system = skr::resource::GetResourceSystem();
    registry = SkrNew<skr::resource::SLocalResourceRegistry>(proj.resource_vfs);
    resource_system->Initialize(registry, proj.ram_service);

    // shader options factory
    {
        skr::renderer::SShaderOptionsFactory::Root factoryRoot = {};
        shaderOptionsFactory = skr::renderer::SShaderOptionsFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderOptionsFactory);
    }
    // shader resource factory
    {
        skr::renderer::SShaderResourceFactory::Root factoryRoot = {};
        factoryRoot.dont_create_shader = true;
        shaderResourceFactory = skr::renderer::SShaderResourceFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderResourceFactory);
    }
    // material type factory
    {
        skr::renderer::SMaterialTypeFactory::Root factoryRoot = {};
        matTypeFactory = skr::renderer::SMaterialTypeFactory::Create(factoryRoot);
        resource_system->RegisterFactory(matTypeFactory);
    }
    {
        skelFactory = SkrNew<skr::resource::SSkelFactory>();
        resource_system->RegisterFactory(skelFactory);
    }
}

void DestroyResourceSystem(skd::SProject& proj)
{
    skr::renderer::SMaterialTypeFactory::Destroy(matTypeFactory);
    skr::renderer::SShaderOptionsFactory::Destroy(shaderOptionsFactory);
    skr::renderer::SShaderResourceFactory::Destroy(shaderResourceFactory);

    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);
}

skr::Vector<skd::SProject*> open_projects(int argc, char** argv)
{
    skr::cmd::parser parser(argc, argv);
    parser.add(u8"project", u8"project path", u8"-p", false);
    parser.add(u8"workspace", u8"workspace path", u8"-w", true);
    if(!parser.parse())
    {
        SKR_LOG_ERROR(u8"Failed to parse command line arguments.");
        return {};
    }
    auto projectPath = parser.get_optional<skr::String>(u8"project");
    std::error_code ec = {};
    skr::filesystem::path workspace{parser.get<skr::String>(u8"workspace").u8_str()};
    skd::SProject::SetWorkspace(workspace);
    skr::filesystem::recursive_directory_iterator iter(workspace, ec);
    skr::stl_vector<skr::filesystem::path> projectFiles;
    while (iter != end(iter))
    {
        if(iter->is_regular_file(ec) && iter->path().extension() == ".sproject")
        {
            projectFiles.push_back(iter->path());
        }
        iter.increment(ec);
    }
    skr::Vector<skd::SProject*> result;
    for (auto& projectFile : projectFiles)
    {
        if(auto proj = skd::SProject::OpenProject(projectFile))
            result.add(proj);
    }
    return result;
}

int compile_project(skd::SProject* project)
{
    auto& system = *skd::asset::GetCookSystem();
    InitializeResourceSystem(*project);
    std::error_code ec = {};
    skr::filesystem::recursive_directory_iterator iter(project->GetAssetPath(), ec);
    //----- scan project directory
    skr::stl_vector<skr::filesystem::path> paths;
    while (iter != end(iter))
    {
        if (iter->is_regular_file(ec) && IsAsset(iter->path()))
        {
            paths.push_back(*iter);
            SKR_LOG_FMT_DEBUG(u8"{}", iter->path().u8string().c_str());
        }
        iter.increment(ec);
    }
    SKR_LOG_INFO(u8"Project dir scan finished.");
    //----- load project asset meta data (guid & type & path)
    {
        using iter_t = typename decltype(paths)::iterator;
        skr::parallel_for(paths.begin(), paths.end(), 20,
        [&](iter_t begin, iter_t end) {
            SkrZoneScopedN("LoadMeta");
            for (auto i = begin; i != end; ++i)
            {
                const auto relpath = skr::filesystem::relative(*i, project->GetAssetPath());
                system.LoadAssetMeta(project, skr::String(relpath.u8string().c_str()));
            }
        });
    }
    SKR_LOG_INFO(u8"Project asset import finished.");
    skr::filesystem::create_directories(project->GetOutputPath(), ec);
    skr::filesystem::create_directories(project->GetDependencyPath(), ec);
    //----- schedule cook tasks (checking dependencies)
    {
        system.ParallelForEachAsset(1,
        [&](skr::span<skd::asset::SAssetRecord*> assets) {
            SkrZoneScopedN("Cook");
            for (auto asset : assets)
            {
                system.EnsureCooked(asset->guid);
            }
        });
    }
    SKR_LOG_INFO(u8"Project asset import finished.");
    auto resource_system = skr::resource::GetResourceSystem();
    skr::task::schedule([&]
    {
        system.WaitForAll();
        resource_system->Quit();
    }, nullptr);
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
    auto& system = *skd::asset::GetCookSystem();
    system.Initialize();
    //----- register project
    auto projects = open_projects(argc, argv);
    SKR_DEFER({ 
        for(auto& project : projects)
            SkrDelete(project); 
    });
    for(auto& project : projects)
        compile_project(project);
    
    scheduler.unbind();
    system.Shutdown();

    return 0;
}

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    {
        FrameMark;
        SkrZoneScopedN("Initialize");
        moduleManager->mount(root.u8string().c_str());
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