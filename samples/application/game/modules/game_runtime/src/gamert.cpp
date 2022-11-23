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
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/mesh_resource.h"
#include "SkrRenderer/resources/shader_resource.hpp"
#include "SkrRenderer/resources/material_resource.hpp"

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
    ioServiceDesc.sleep_mode = SKR_ASYNC_SERVICE_SLEEP_MODE_SLEEP;
    ioServiceDesc.sleep_time = 1000 / 60;
    ioServiceDesc.lockless = true;
    ioServiceDesc.sort_method = SKR_ASYNC_SERVICE_SORT_METHOD_PARTIAL;
    ram_service = skr::io::RAMService::create(&ioServiceDesc);

    registry = SkrNew<skr::resource::SLocalResourceRegistry>(resource_vfs);
    skr::resource::GetResourceSystem()->Initialize(registry, ram_service);
    // 
    using namespace skr::guid::literals;
    auto resource_system = skr::resource::GetResourceSystem();
    
    auto gameResourceRoot = resourceRoot / "game";
    auto u8TextureRoot = gameResourceRoot.u8string();
    // texture factory
    {
        skr_vfs_desc_t tex_vfs_desc = {};
        tex_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
        tex_vfs_desc.override_mount_dir = u8TextureRoot.c_str();
        tex_resource_vfs = skr_create_vfs(&tex_vfs_desc);

        skr::resource::STextureFactory::Root factoryRoot = {};
        factoryRoot.dstorage_root = gameResourceRoot;
        factoryRoot.texture_vfs = tex_resource_vfs;
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = game_render_device->get_vram_service();
        factoryRoot.render_device = game_render_device;
        textureFactory = skr::resource::STextureFactory::Create(factoryRoot);
        resource_system->RegisterFactory("f8821efb-f027-4367-a244-9cc3efb3a3bf"_guid, textureFactory);
    }
    // mesh factory
    {
        skr::resource::SMeshFactory::Root factoryRoot = {};
        factoryRoot.dstorage_root = gameResourceRoot;
        factoryRoot.texture_vfs = tex_resource_vfs;
        factoryRoot.ram_service = ram_service;
        factoryRoot.vram_service = game_render_device->get_vram_service();
        factoryRoot.render_device = game_render_device;
        meshFactory = skr::resource::SMeshFactory::Create(factoryRoot);
        resource_system->RegisterFactory("3b8ca511-33d1-4db4-b805-00eea6a8d5e1"_guid, meshFactory);
    }
    // shader factory
    {
        const auto backend = game_render_device->get_backend();
        std::string shaderType = "invalid";
        if (backend == CGPU_BACKEND_D3D12) shaderType = "dxil";
        if (backend == CGPU_BACKEND_VULKAN) shaderType = "spirv";
        auto shaderResourceRoot = gameResourceRoot / shaderType;
        auto u8ShaderResourceRoot = shaderResourceRoot.u8string();

        skr_vfs_desc_t shader_vfs_desc = {};
        shader_vfs_desc.mount_type = SKR_MOUNT_TYPE_CONTENT;
        shader_vfs_desc.override_mount_dir = u8ShaderResourceRoot.c_str();
        shader_bytes_vfs = skr_create_vfs(&shader_vfs_desc);

        skr::resource::SShaderResourceFactory::Root factoryRoot = {};
        factoryRoot.bytecode_vfs = shader_bytes_vfs;
        factoryRoot.ram_service = ram_service;
        factoryRoot.render_device = game_render_device;
        factoryRoot.aux_service = game_render_device->get_aux_service(0);
        shaderFactory = skr::resource::SShaderResourceFactory::Create(factoryRoot);
        resource_system->RegisterFactory("1c7d845a-fde8-4487-b1c9-e9c48d6a9867"_guid, shaderFactory);
    }

    skr_resource_handle_t shaderHdl("0c11a646-93ec-4cd8-8bc4-72c1aca8ec57"_guid);
    shaderHdl.resolve(true, 0, SKR_REQUESTER_SYSTEM);
    // texture
    {
        while (shaderHdl.get_status() != SKR_LOADING_STATUS_INSTALLED && shaderHdl.get_status() != SKR_LOADING_STATUS_ERROR)
        {
            resource_system->Update();
        }
        auto final_status = shaderHdl.get_status();
        if (final_status != SKR_LOADING_STATUS_ERROR)
        {
            auto shader = (skr_platform_shader_resource_t*)shaderHdl.get_ptr();
            SKR_LOG_TRACE("Shader Loaded: entries - %d", shader->shader->entrys_count);
            resource_system->UnloadResource(shaderHdl);
            resource_system->Update();
            while (shaderHdl.get_status(true) != SKR_LOADING_STATUS_UNLOADED)
            {
                resource_system->Update();
            }
        }
    }
    /*
    {
        skr_json_writer_t writer(2);
        auto mat = make_zeroed<skr_material_value_t>();
        mat.value = 1.f;
        skr::json::Write(&writer, mat);
        auto str = writer.buffer.data();
        SKR_LOG_DEBUG("JSON:\n %s", str);
    }
    */
}

void SGameRTModule::on_unload()
{
    skr::resource::STextureFactory::Destroy(textureFactory);
    skr::resource::SMeshFactory::Destroy(meshFactory);
    skr::resource::SShaderResourceFactory::Destroy(shaderFactory);

    dualS_release(game_world);
    skr_free_renderer(game_renderer);
    
    skr::resource::GetResourceSystem()->Shutdown();
    SkrDelete(registry);

    skr::io::RAMService::destroy(ram_service);
    skr_free_vfs(resource_vfs);
    skr_free_vfs(tex_resource_vfs);
    skr_free_vfs(shader_bytes_vfs);

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