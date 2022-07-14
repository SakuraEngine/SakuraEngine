#include "asset/cooker.hpp"
#include "asset/importer.hpp"
#include "bitsery/adapter/buffer.h"
#include "ecs/dual.h"
#include "ftl/task_counter.h"
#include "ghc/filesystem.hpp"
#include "platform/guid.h"
#include "platform/thread.h"
#include "resource/config_resource.h"
#include "resource/resource_header.h"
#include <mutex>
#include "asset/config_asset.hpp"
#include "bitsery/serializer.h"
#include "tracy/Tracy.hpp"
#include "type/type_registry.h"
#include "utils/defer.hpp"
#include "resource/resource_header.h"
#include "utils/format.hpp"
#include "module/module_manager.hpp"
#include "platform/vfs.h"
#include <string>
#include <vector>
#include "utils/log.h"

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

bool IsAsset(ghc::filesystem::path path)
{
    if (path.extension() == ".meta")
        return true;
    if (path.extension() != ".asset")
    {
        auto metaPath = path.string() + ".asset";
        if (!ghc::filesystem::exists(metaPath)) // skip asset without meta
            return false;
        return true;
    }
    return false;
}

int main(int argc, char** argv)
{
    FrameMark;
    auto moduleManager = skr_get_module_manager();
    auto root = ghc::filesystem::current_path();
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph("GameTool", true);
    moduleManager->init_module_graph(argc, argv);
    #ifdef WITH_USDTOOL
    moduleManager->patch_module_graph("UsdTool", true);
    #endif
    auto& system = *skd::asset::GetCookSystem();
    system.Initialize();
    dualJ_initialize((dual_scheduler_t*)&system.GetScheduler());
    //----- register project
    // TODO: project discover?
    auto project = SkrNew<skd::asset::SProject>();
    SKR_DEFER({ SkrDelete(project); });
    auto parentPath = root.parent_path().u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.app_name = "Project";
    vfs_desc.mount_type = SKR_MOUNT_TYPE_ABSOLUTE;
    vfs_desc.override_mount_dir = parentPath.c_str();
    project->vfs = skr_create_vfs(&vfs_desc);
    project->assetPath = (root.parent_path() / "../../../samples/application/game/assets").lexically_normal();
    project->outputPath = (root.parent_path() / "resources/game").lexically_normal();
    project->dependencyPath = (root.parent_path() / "deps/game").lexically_normal();

    ghc::filesystem::recursive_directory_iterator iter(project->assetPath);
    //----- scan project directory
    eastl::vector<ghc::filesystem::path> paths;
    for (auto& entry : iter)
        if (entry.is_regular_file() && IsAsset(entry.path()))
            paths.push_back(*iter);
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
    ghc::filesystem::create_directories(project->outputPath);
    ghc::filesystem::create_directories(project->dependencyPath);
    //----- schedule cook tasks (checking dependencies)
    {
        using iter_t = typename decltype(system.assets)::iterator;
        system.ParallelFor(system.assets.begin(), system.assets.end(), 10,
        [](iter_t begin, iter_t end) {
            ZoneScopedN("EnsureCooked");
            auto& system = *skd::asset::GetCookSystem();
            for (auto i = begin; i != end; ++i)
                if (!(i->second->type == skr_guid_t{}))
                    system.EnsureCooked(i->second->guid);
        });
    }
    SKR_LOG_INFO("Project asset import finished.");
    //----- wait
    system.WaitForAll();
    system.Shutdown();
    moduleManager->destroy_module_graph();
    return 0;
}