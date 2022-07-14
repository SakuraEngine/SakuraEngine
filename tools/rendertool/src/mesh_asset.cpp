#include "ecs/dual.h"
#include "utils/io.hpp"
#include "mesh_asset.hpp"
#include "mesh-resource.h"
#include "resource/resource_factory.h"
#include "utils/defer.hpp"
#include "utils/log.h"

void* skd::asset::SGltfMeshImporter::Import(skr::io::RAMService* ioService, const skd::asset::SAssetRecord* record) 
{
    auto ext = record->path.extension();
    if (ext != ".gltf")
    {
        return nullptr;
    }
    // prepare callback
    auto u8Path = record->path.u8string();
    ftl::AtomicFlag counter(&GetCookSystem()->GetScheduler());
    struct CallbackData
    {
        ftl::AtomicFlag* pCounter;   
        eastl::string u8Path;
    } callbackData;
    callbackData.pCounter = &counter;
    callbackData.u8Path = u8Path.c_str();
    counter.Set();
    // prepare io
    skr_ram_io_t ramIO = {};
    ramIO.bytes = nullptr;
    ramIO.offset = 0;
    ramIO.size = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cgltf_options options = {};
        struct cgltf_data* gltf_data_ = nullptr;
        if (request->bytes)
        {
            cgltf_result result = cgltf_parse(&options, request->bytes, request->size, &gltf_data_);
            if (result != cgltf_result_success)
            {
                gltf_data_ = nullptr;
            }
            else
            {
                result = cgltf_load_buffers(&options, gltf_data_, cbData->u8Path.c_str());
                result = cgltf_validate(gltf_data_);
                if (result != cgltf_result_success)
                {
                    gltf_data_ = nullptr;
                }
            }
        }
        sakura_free(request->bytes);
        request->bytes = (uint8_t*)gltf_data_;
        request->size = 114514;
        cbData->pCounter->Clear();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    skr_async_io_request_t ioRequest = {};
    ioService->request(record->project->vfs, &ramIO, &ioRequest);
    GetCookSystem()->scheduler->WaitForCounter(&counter, true);
    // parse
    if (ioRequest.size == 114514)
    {
        cgltf_data* data = (cgltf_data*)ioRequest.bytes;
        auto mesh = new skr_mesh_resource_t();
        mesh->name = data->meshes[0].name;
        cgltf_free(data);
        return mesh;
    }
    return nullptr;
}

bool skd::asset::SMeshCooker::Cook(SCookContext * ctx)
{ 
    auto resource = ctx->Import<skr_mesh_resource_t>();
    //-----write resource header
    eastl::vector<uint8_t> buffer;
    skr::resource::SBinarySerializer archive(buffer);
    ctx->WriteHeader(archive, this);
    //------write resource object

    //------save resource to disk
    auto file = fopen(ctx->output.u8string().c_str(), "wb");
    if (!file)
    {
        SKR_LOG_FMT_ERROR("[SConfigCooker::Cook] failed to write cooked file for resource {}! path: {}", ctx->record->guid, ctx->record->path.u8string());
        return false;
    }
    SKR_DEFER({ fclose(file); });
    fwrite(buffer.data(), 1, archive.adapter().writtenBytesCount(), file);
    return true;
}

uint32_t skd::asset::SMeshCooker::Version() 
{ 
    return 0; 
}