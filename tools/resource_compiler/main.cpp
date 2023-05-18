#include "misc/parallel_for.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "ecs/dual.h"
#include "platform/filesystem.hpp"
#include "platform/thread.h"
#include "resource/config_resource.h"
#include "resource/resource_header.hpp"
#include "SkrToolCore/assets/config_asset.hpp"
#include "type/type.hpp"
#include "misc/defer.hpp"
#include "resource/resource_header.hpp"
#include "module/module_manager.hpp"
#include "platform/vfs.h"
#include "misc/log.h"
#include "misc/log.hpp"
#include "misc/io.h"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/material_type_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"
#include "misc/make_zeroed.hpp"
#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/resources/animation_resource.h"

#include "containers/string.hpp"

#include "tracy/Tracy.hpp"

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
    using namespace skr::guid::literals;
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

skd::SProject* open_project(int argc, char** argv)
{
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    auto parentPath = root.parent_path().u8string();
    auto project = SkrNew<skd::SProject>();
    project->assetPath = (root.parent_path() / "../../../samples/application/game/assets").lexically_normal();
    project->outputPath = (root.parent_path() / "resources/game").lexically_normal();
    project->dependencyPath = (root.parent_path() / "deps/game").lexically_normal();

    // create VFS
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.app_name = u8"Project";
    vfs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    vfs_desc.override_mount_dir = parentPath.c_str();
    project->vfs = skr_create_vfs(&vfs_desc);

    // create resource VFS
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources");
    auto u8ResourceRoot = resourceRoot.u8string();
    skr_vfs_desc_t resource_vfs_desc = {};
    resource_vfs_desc.app_name = u8"Project";
    resource_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    resource_vfs_desc.override_mount_dir = u8ResourceRoot.c_str();

    project->resource_vfs = skr_create_vfs(&resource_vfs_desc);
    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = u8"CompilerRAMIOService";
    ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
    project->ram_service = skr_io_ram_service_t::create(&ioServiceDesc);
    return project;
}

int compile_all(int argc, char** argv)
{
    log_set_level(SKR_LOG_LEVEL_INFO);
    
    skr::task::scheduler_t scheduler;
    scheduler.initialize(skr::task::scheudler_config_t());
    scheduler.bind();
    auto& system = *skd::asset::GetCookSystem();
    system.Initialize();
    //----- register project
    // TODO: project discover?
    auto project = open_project(argc, argv);
    SKR_DEFER({ SkrDelete(project); });
    
    InitializeResourceSystem(*project);

    std::error_code ec = {};
    skr::filesystem::recursive_directory_iterator iter(project->assetPath, ec);
    //----- scan project directory
    eastl::vector<skr::filesystem::path> paths;
    while (iter != end(iter))
    {
        if (iter->is_regular_file(ec) && IsAsset(iter->path()))
        {
            paths.push_back(*iter);
            SKR_LOG_FMT_DEBUG(u8"{}", iter->path().u8string().c_str());
        }
        iter.increment(ec);
    }
    SKR_LOG_INFO("Project dir scan finished.");
    //----- import project assets (guid & type & path)
    {
        using iter_t = typename decltype(paths)::iterator;
        skr::parallel_for(paths.begin(), paths.end(), 20,
        [&](iter_t begin, iter_t end) {
            ZoneScopedN("Import");
            for (auto i = begin; i != end; ++i)
                system.ImportAsset(project, *i);
        });
    }
    SKR_LOG_INFO("Project asset import finished.");
    skr::filesystem::create_directories(project->outputPath, ec);
    skr::filesystem::create_directories(project->dependencyPath, ec);
    //----- schedule cook tasks (checking dependencies)
    {
        system.ParallelForEachAsset(1,
        [&](skr::span<skd::asset::SAssetRecord*> assets) {
            ZoneScopedN("Cook");
            for (auto asset : assets)
            {
                system.EnsureCooked(asset->guid);
            }
        });
    }
    SKR_LOG_INFO("Project asset import finished.");
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
    scheduler.unbind();
    system.Shutdown();
    DestroyResourceSystem(*project);
    return 0;
}

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    {
        FrameMark;
        ZoneScopedN("Initialize");
        moduleManager->mount(root.u8string().c_str());
        moduleManager->make_module_graph(u8"SkrResourceCompiler", true);
        moduleManager->init_module_graph(argc, argv);
    }
    {
        FrameMark;
        ZoneScopedN("CompileAll");
        compile_all(argc, argv);
    }
    {
        FrameMark;
        ZoneScopedN("ThreadExit");
        moduleManager->destroy_module_graph();
    }
    return 0;
}