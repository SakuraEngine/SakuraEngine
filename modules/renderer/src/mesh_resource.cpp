#include "skr_renderer/mesh_resource.h"
#include "platform/memory.h"
#include "utils/io.hpp"
#include "utils/make_zeroed.hpp"
#include "ftl/atomic_counter.h"
#include "skr_renderer/cgltf.h"

void skr_mesh_resource_create_from_gltf(skr_io_ram_service_t* ioService, const char* path, skr_gltf_ram_io_request_t* request)
{
    SKR_ASSERT(request->vfs_override && "Support only vfs override");
    struct CallbackData
    {
        skr_gltf_ram_io_request_t* gltfRequest;   
        eastl::string u8Path;
    } callbackData;
    skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
    ramIO.bytes = nullptr;
    ramIO.offset = 0;
    ramIO.size = 0;
    ramIO.path = path;
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
                else
                {
                    skr_mesh_resource_id resource = (skr_mesh_resource_id)sakura_calloc(1, sizeof(skr_mesh_resource_t));
                    // TODO: fill runtime structure
                }
            }
        }
        sakura_free(request->bytes);
        request->bytes = nullptr;
        request->size = 0;
        skr_atomic32_store_relaxed(&cbData->gltfRequest->gltf_status, SKR_ASYNC_IO_STATUS_OK);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    // pass status
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_ENQUEUED] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        skr_atomic32_store_relaxed(&cbData->gltfRequest->gltf_status, SKR_ASYNC_IO_STATUS_ENQUEUED);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_ENQUEUED] = (void*)&callbackData;
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_RAM_LOADING] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        skr_atomic32_store_relaxed(&cbData->gltfRequest->gltf_status, SKR_ASYNC_IO_STATUS_RAM_LOADING);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_RAM_LOADING] = (void*)&callbackData;
    
    ioService->request(request->vfs_override, &ramIO, &request->ioRequest);
}