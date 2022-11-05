#include "gamert.h"
#include "utils/make_zeroed.hpp"
#include "platform/configure.h"
#include "platform/filesystem.hpp"
#include "platform/memory.h"
#include "resource/resource_system.h"
#include "resource/local_resource_registry.h"
#include "ecs/dual.h"
#include "runtime_module.h"
#include "platform/guid.hpp"
#include "skr_renderer/resources/texture_resource.h"
#include "skr_renderer/resources/mesh_resource.h"

IMPLEMENT_DYNAMIC_MODULE(SGameRTModule, GameRT);


void SGameRTModule::on_load(int argc, char** argv)
{
    SKR_LOG_INFO("game runtime loaded!");

    // create game world
    game_world = dualS_create();
    
    game_render_device = skr_get_default_render_device();
    game_renderer = skr_create_renderer(game_render_device, game_world);

    std::error_code ec = {};
    auto resourceRoot = (skr::filesystem::current_path(ec) / "../resources");
    auto u8ResourceRoot = resourceRoot.u8string();
    skr_vfs_desc_t vfs_desc = {};
    vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    vfs_desc.override_mount_dir = u8ResourceRoot.c_str();
    resource_vfs = skr_create_vfs(&vfs_desc);

    auto ioServiceDesc = make_zeroed<skr_ram_io_service_desc_t>();
    ioServiceDesc.name = "GameRuntimeRAMIOService";
    ioServiceDesc.sleep_mode = SKR_IO_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_IO_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr::io::RAMService::create(&ioServiceDesc);

    registry = SkrNew<skr::resource::SLocalResourceRegistry>(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(registry, ram_service);
    // 
    using namespace skr::guid::literals;
    auto resource_system = skr::resource::GetResourceSystem();
    
    auto textureRoot = resourceRoot / "game";
    auto u8TextureRoot = textureRoot.u8string();

    skr_vfs_desc_t tex_vfs_desc = {};
    tex_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
    tex_vfs_desc.override_mount_dir = u8TextureRoot.c_str();
    tex_resource_vfs = skr_create_vfs(&tex_vfs_desc);

    // texture factory
    skr::resource::STextureFactory* textureFactory = nullptr;
    {
        skr::resource::STextureFactory::Root factoryRoot = {};
        factoryRoot.dstorage_root = textureRoot;
        factoryRoot.texture_vfs = tex_resource_vfs;
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = game_render_device->get_vram_service();
        factoryRoot.render_device = game_render_device;
        textureFactory = skr::resource::STextureFactory::Create(factoryRoot);
        resource_system->RegisterFactory("f8821efb-f027-4367-a244-9cc3efb3a3bf"_guid, textureFactory);
    }
    // mesh factory
    skr::resource::SMeshFactory* meshFactory = nullptr;
    {
        skr::resource::SMeshFactory::Root factoryRoot = {};
        meshFactory = skr::resource::SMeshFactory::Create(factoryRoot);
        resource_system->RegisterFactory("3b8ca511-33d1-4db4-b805-00eea6a8d5e1"_guid, meshFactory);
    }
    // load resources
    skr_resource_handle_t textureHdl("cb5fe6d7-5d91-4f3b-81b0-0a7afbf1a7cb"_guid);
    skr_resource_handle_t gltfHdl("79bb81eb-4e9f-4301-bf0c-a15b10a1cc3b"_guid);
    resource_system->LoadResource(textureHdl);
    resource_system->LoadResource(gltfHdl);
    // texture
    {
        while (textureHdl.get_status() != SKR_LOADING_STATUS_INSTALLED && textureHdl.get_status() != SKR_LOADING_STATUS_ERROR)
        {
            resource_system->Update();
        }
        auto final_status = textureHdl.get_status();
        if (final_status != SKR_LOADING_STATUS_ERROR)
        {
            auto texture = (skr_texture_resource_t*)textureHdl.get_ptr();
            SKR_LOG_TRACE("Texture Loaded: format - %d, mips - %d, data size - %d", 
                texture->format, texture->mips_count, texture->data_size);
            resource_system->UnloadResource(textureHdl);
            resource_system->Update();
            while (textureHdl.get_status() != SKR_LOADING_STATUS_UNLOADED)
            {
                resource_system->Update();
            }
        }
    }
    // mesh
    {
        while (gltfHdl.get_status() != SKR_LOADING_STATUS_INSTALLED && gltfHdl.get_status() != SKR_LOADING_STATUS_ERROR)
        {
            resource_system->Update();
        }
        auto final_status = gltfHdl.get_status();
        if (final_status != SKR_LOADING_STATUS_ERROR)
        {
            auto mesh = (skr_mesh_resource_id)gltfHdl.get_ptr();
            SKR_LOG_TRACE("Mesh Loaded: name - %s, bin0 - %s", mesh->name.c_str(), mesh->bins[0].uri.c_str());
            resource_system->UnloadResource(gltfHdl);
            resource_system->Update();
            while (gltfHdl.get_status() != SKR_LOADING_STATUS_UNLOADED)
            {
                resource_system->Update();
            }
        }
    }
    skr::resource::STextureFactory::Destroy(textureFactory);
    skr::resource::SMeshFactory::Destroy(meshFactory);
}

void SGameRTModule::on_unload()
{
    dualS_release(game_world);
    skr_free_renderer(game_renderer);
    
    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);

    skr::io::RAMService::destroy(ram_service);
    skr_free_vfs(resource_vfs);
    skr_free_vfs(tex_resource_vfs);

    SKR_LOG_INFO("game runtime unloaded!");
}

SGameRTModule* SGameRTModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<SGameRTModule*>(mm->get_module("GameRT"));
    return rm;
}

skr_vfs_t* skr_game_runtime_get_vfs()
{
    return SGameRTModule::Get()->resource_vfs;
}


skr_io_ram_service_t* skr_game_runtime_get_ram_service()
{
     return SGameRTModule::Get()->ram_service;
}

dual_storage_t* skr_game_runtime_get_world()
{
    return SGameRTModule::Get()->game_world;
}

SRenderDeviceId skr_game_runtime_get_render_device()
{
    return SGameRTModule::Get()->game_render_device;
}

SRendererId skr_game_runtime_get_renderer()
{
    return SGameRTModule::Get()->game_renderer;
}