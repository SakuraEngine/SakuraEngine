#include "SkrGLTFTool/gltf_utils.hpp"
#include "resource/resource_factory.h"
#include "SkrToolCore/project/project.hpp"
#include "SkrGLTFTool/mesh_asset.hpp"

#define MAGIC_SIZE_GLTF_PARSE_READY ~0

namespace skd
{
namespace asset
{
cgltf_data* ImportGLTFWithData(skr::string_view assetPath, skr::io::RAMService* ioService, struct SCookContext* context) SKR_NOEXCEPT
{
    skr::filesystem::path relPath = assetPath.data();
    const auto assetRecord = context->GetAssetRecord();
    auto ext = relPath.extension();
    if (ext != ".gltf")
    {
        return nullptr;
    }
    auto path = context->AddFileDependency(relPath);
    // prepare callback
    auto u8Path = path.u8string();
    skr::task::event_t counter;
    struct CallbackData
    {
        skr_async_ram_destination_t destination;
        skr::task::event_t* pCounter;   
        skr::string u8Path;
    } callbackData;
    callbackData.pCounter = &counter;
    callbackData.u8Path = u8Path.c_str();
    // prepare io
    skr_ram_io_t ramIO = {};
    ramIO.offset = 0;
    ramIO.path = u8Path.c_str();
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_request_t* request, void* data) noexcept {
        auto cbData = (CallbackData*)data;
        cgltf_options options = {};
        struct cgltf_data* gltf_data_ = nullptr;
        if (cbData->destination.bytes)
        {
            cgltf_result result = cgltf_parse(&options, cbData->destination.bytes, cbData->destination.size, &gltf_data_);
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
        sakura_free(cbData->destination.bytes);
        cbData->destination.bytes = (uint8_t*)gltf_data_;
        cbData->destination.size = MAGIC_SIZE_GLTF_PARSE_READY;
        cbData->pCounter->signal();
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)&callbackData;
    skr_async_request_t ioRequest = {};
    ioService->request(assetRecord->project->vfs, &ramIO, &ioRequest, &callbackData.destination);
    counter.wait(false);
    // parse
    if (callbackData.destination.size == MAGIC_SIZE_GLTF_PARSE_READY)
    {
        cgltf_data* gltf_data = (cgltf_data*)callbackData.destination.bytes;
        return gltf_data;
    }
    return nullptr;
}
} // namespace asset
} // namespace skd