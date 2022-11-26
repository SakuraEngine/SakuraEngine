#include "SkrToolCore/project/project.hpp"
#include "SkrToolCore/asset/cook_system.hpp"
#include "SkrToolCore/asset/importer.hpp"
#include "ecs/dual.h"
#include "platform/filesystem.hpp"
#include "platform/thread.h"
#include "resource/config_resource.h"
#include "resource/resource_header.hpp"
#include <mutex>
#include "SkrToolCore/assets/config_asset.hpp"
#include "type/type_registry.h"
#include "utils/defer.hpp"
#include "resource/resource_header.hpp"
#include "utils/format.hpp"
#include "module/module_manager.hpp"
#include "platform/vfs.h"
#include "utils/log.h"
#include "utils/log.hpp"
#include "utils/io.hpp"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/shader_meta_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"
#include "utils/make_zeroed.hpp"
#include "SkrAnim/resources/skeleton_resource.h"
#include "SkrAnim/resources/animation_resource.h"

#include "tracy/Tracy.hpp"

/*
#include "google/protobuf/empty.pb.h"

#include "skrcompiler.grpc.pb.h"
#include "skrcompiler.pb.h"
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/status.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/create_channel.h>

class CompileResourceImpl final : public skrcompiler::CompileResource::Service
{
    ::grpc::Status Compile(::grpc::ServerContext* context, const ::skrcompiler::CompileInfo* request, ::skrcompiler::CompileResult* response) override
    {
        using namespace grpc;
        response->set_errorcode(0);
        response->set_errormessage("fuck");
        (void)stub;

        return Status::OK;
    }
    skrcompiler::HostResource::Stub* stub;
};

class HostResourceImpl final : public skrcompiler::HostResource::Service
{
    ::grpc::Status GetPath(::grpc::ServerContext* context, const ::skrcompiler::ResourceId* request, ::skrcompiler::ResourcePath* response) override
    {
        using namespace grpc;
        response->set_path(request->guid() + "asdsad");
        return Status::OK;
    }

    ::grpc::Status Register(::grpc::ServerContext* context, const ::skrcompiler::Port* request, ::google::protobuf::Empty* response) override
    {
        using namespace grpc;
        std::cout << "Worker registering on localhost:" << request->number() << std::endl;
        stub.push_back(std::make_unique<skrcompiler::CompileResource::Stub>(CreateChannel("localhost:" + request->number(), InsecureChannelCredentials())));

        std::cout << "Worker connected on localhost:" << request->number() << std::endl;
        return Status::OK;
    }
    std::vector<std::unique_ptr<skrcompiler::CompileResource::Stub>> stub;
};

void Run(bool server)
{
    using namespace grpc;
    if (!server)
    {
        std::string server_address1("localhost:0");
        CompileResourceImpl service;
        ServerBuilder builder;
        int selectedPort = 0;
        std::stringstream ss;
        builder.AddListeningPort(server_address1, grpc::InsecureServerCredentials(), &selectedPort);
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        ss << selectedPort;
        std::cout << "Server listening on localhost:" << ss.str() << std::endl;
        skrcompiler::HostResource::Stub stub(CreateChannel("localhost:50052", InsecureChannelCredentials()));
        std::cout << "connected to localhost:50052" << std::endl;
        ClientContext context;
        grpc::CompletionQueue cq;
        skrcompiler::Port id;
        id.set_number(ss.str());
        google::protobuf::Empty empty;
        auto reg = stub.Register(&context, id, &empty);
        std::cout << "registed to localhost:50052" << std::endl;
        server->Wait();
    }
    else
    {
        std::string server_address("localhost:50052");
        HostResourceImpl service;
        ServerBuilder builder;
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;
        server->Wait();
    }
}
*/

bool IsAsset(skr::filesystem::path path)
{
    if (path.extension() == ".meta")
        return true;
    return false;
}

skr::resource::SShaderOptionsFactory* shaderOptionsFactory = nullptr;
skr::resource::SSkelFactory* skelFactory = nullptr;
skr::resource::SLocalResourceRegistry* registry = nullptr;

void InitializeResourceSystem(skd::SProject& proj)
{
    using namespace skr::guid::literals;
    auto resource_system = skr::resource::GetResourceSystem();
    registry = SkrNew<skr::resource::SLocalResourceRegistry>(proj.resource_vfs);
    resource_system->Initialize(registry, proj.ram_service);

    // shader options factory
    {
        skr::resource::SShaderOptionsFactory::Root factoryRoot = {};
        shaderOptionsFactory = skr::resource::SShaderOptionsFactory::Create(factoryRoot);
        resource_system->RegisterFactory(shaderOptionsFactory);
    }

    {
        skelFactory = SkrNew<skr::resource::SSkelFactory>();
        resource_system->RegisterFactory(skelFactory);
    }
}

void DestroyResourceSystem(skd::SProject& proj)
{
    skr::resource::SShaderOptionsFactory::Destroy(shaderOptionsFactory);

    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);
}

int main(int argc, char** argv)
{
    log_set_level(SKR_LOG_LEVEL_INFO);

    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("SkrResourceCompiler", true);
    moduleManager->init_module_graph(argc, argv);

    FrameMark;
    ZoneScopedN("CookAll");

    skr::task::scheduler_t scheduler;
    scheduler.initialize(skr::task::scheudler_config_t());
    scheduler.bind();
    auto& system = *skd::asset::GetCookSystem();
    system.Initialize();
    //----- register project
    // TODO: project discover?
    auto project = SkrNew<skd::SProject>();
    SKR_DEFER({ SkrDelete(project); });
    auto parentPath = root.parent_path().u8string();

    project->assetPath = (root.parent_path() / "../../../samples/application/game/assets").lexically_normal();
    project->outputPath = (root.parent_path() / "resources/game").lexically_normal();
    project->dependencyPath = (root.parent_path() / "deps/game").lexically_normal();

    // create VFS
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.app_name = "Project";
    vfs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    vfs_desc.override_mount_dir = parentPath.c_str();
    project->vfs = skr_create_vfs(&vfs_desc);

    // create resource VFS
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources");
    auto u8ResourceRoot = resourceRoot.u8string();
    skr_vfs_desc_t resource_vfs_desc = {};
    resource_vfs_desc.app_name = "Project";
    resource_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    resource_vfs_desc.override_mount_dir = u8ResourceRoot.c_str();

    project->resource_vfs = skr_create_vfs(&resource_vfs_desc);
    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = "GameRuntimeRAMIOService";
    ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
    project->ram_service = skr::io::RAMService::create(&ioServiceDesc);
    InitializeResourceSystem(*project);

    skr::filesystem::recursive_directory_iterator iter(project->assetPath, ec);
    //----- scan project directory
    eastl::vector<skr::filesystem::path> paths;
    while (iter != end(iter))
    {
        if (iter->is_regular_file(ec) && IsAsset(iter->path()))
        {
            paths.push_back(*iter);
            SKR_LOG_FMT_DEBUG("{}", iter->path().u8string());
        }
        iter.increment(ec);
    }
    SKR_LOG_INFO("Project dir scan finished.");
    //----- import project assets (guid & type & path)
    {
        using iter_t = typename decltype(paths)::iterator;
        system.ParallelFor(paths.begin(), paths.end(), 20,
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
        const auto& assetMap = system.GetAssetMap();
        using iter_t = typename std::decay_t<decltype(assetMap)>::const_iterator;
        system.ParallelFor(assetMap.begin(), assetMap.end(), 1,
        [](iter_t begin, iter_t end) {
            ZoneScopedN("EnsureCooked");
            auto& system = *skd::asset::GetCookSystem();
            for (auto i = begin; i != end; ++i)
                if (!(i->second->type == skr_guid_t{}))
                    system.EnsureCooked(i->second->guid);
        });
    }
    SKR_LOG_INFO("Project asset import finished.");
    auto resource_system = skr::resource::GetResourceSystem();
    resource_system->Update();
    //----- wait
    while (!system.AllCompleted())
    {
        resource_system->Update();
    }
    scheduler.unbind();
    system.Shutdown();
    DestroyResourceSystem(*project);
    {
        ZoneScopedN("ThreadExit");
        moduleManager->destroy_module_graph();
    }
#ifdef TRACY_ENABLE
    skr_thread_sleep(800);
#endif
    return 0;
}