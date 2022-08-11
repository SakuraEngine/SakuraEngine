#include "platform/debug.h"
#include "platform/memory.h"
#include "utils/make_zeroed.hpp"
#include "utils/log.h"
#include "utils/io.hpp"
#include "platform/vfs.h"

#include <ghc/filesystem.hpp>
#include <EASTL/vector_map.h>
#include <EASTL/string.h>

#include "skr_live2d/model_resource.h"
#include "Framework/CubismFramework.hpp"
#include "Framework/Type/csmMap.hpp"
#include "Framework/Effect/CubismBreath.hpp"
#include "Framework/Model/CubismUserModel.hpp"
#include "Framework/CubismModelSettingJson.hpp"

#include "tracy/Tracy.hpp"

struct L2DRequestCallbackData
{
    skr_io_ram_service_t* ioService;
    skr_live2d_ram_io_request_t* live2dRequest;   
    eastl::string u8HomePath;
    uint32_t expression_count;
    uint32_t model_count;
    SAtomic32 finished_expressions;
    SAtomic32 finished_models;

    void partial_finished()
    {
        auto _e = skr_atomic32_load_acquire(&finished_expressions);
        auto _m = skr_atomic32_load_acquire(&finished_models);
        if (_e == expression_count && _m == model_count)
        {
            skr_atomic32_store_relaxed(&live2dRequest->liv2d_status, SKR_ASYNC_IO_STATUS_OK);
            SkrDelete(this);
        }
    }
};

namespace L2DF = Live2D::Cubism::Framework;

namespace Live2D { namespace Cubism { namespace Framework {
    class csmUserModel : public CubismUserModel
    {
    public:
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data)
        {
            auto settings = data->live2dRequest->model_resource->model_setting;
            auto file = settings->GetModelFileName();
            u8Path = data->u8HomePath + "/" + file;
            cbData = data;

            SKR_LOG_DEBUG("Read Live2D From Home %s", data->u8HomePath.c_str());
            SKR_LOG_DEBUG("Live2D Model %s at %s", file, u8Path.c_str());
            skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
            ramIO.path = u8Path.c_str();
            ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                auto _this = (csmUserModel*)data;
                
                _this->LoadModel(request->bytes, (L2DF::csmSizeInt)request->size);
                sakura_free(request->bytes);
                
                skr_atomic32_add_relaxed(&_this->cbData->finished_models, 1);
                _this->cbData->partial_finished();
            };
            ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)this;
            ioService->request(cbData->live2dRequest->vfs_override, &ramIO, &modelRequest);
        }

        eastl::string u8Path;
        L2DRequestCallbackData* cbData;
        skr_async_io_request_t modelRequest;
    };
    class csmMotionMap : public csmMap<csmString, ACubismMotion*>
    {
    public:
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data)
        {
            cbData = data;

            auto settings = data->live2dRequest->model_resource->model_setting;
            expression_names.resize(settings->GetExpressionCount());
            expression_paths.resize(settings->GetExpressionCount());
            expression_requests.resize(settings->GetExpressionCount());
            for (uint32_t i = 0; i < expression_requests.size(); i++)
            {
                auto& request = expression_requests[i];
                auto&& [pRequest, path] = expression_paths.at(i);
                auto&& [pRequest_, name] = expression_names.at(i);

                name = settings->GetExpressionName(i);
                auto file = settings->GetExpressionFileName(i);
                pRequest = &request;
                pRequest_ = &request;
                path = data->u8HomePath + "/" + file;

                SKR_LOG_DEBUG("Live2D Expression %s at %s", name.c_str(), file);
                skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
                ramIO.path = path.c_str();
                ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                    auto _this = (csmMotionMap*)data;
                    auto name = _this->expression_names[request];
                    csmMap<csmString, ACubismMotion*>& map = *_this;
                    SKR_LOG_DEBUG("Live2D Deserialize Expression %s", name.c_str());
                                
                    sakura_free(request->bytes);
                    
                    skr_atomic32_add_relaxed(&_this->cbData->finished_expressions, 1);
                    _this->cbData->partial_finished();
                };
                ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)this;
                ioService->request(cbData->live2dRequest->vfs_override, &ramIO, &request);
            }
        }
        eastl::vector<skr_async_io_request_t> expression_requests;
        eastl::vector_map<skr_async_io_request_t*, eastl::string> expression_names;
        eastl::vector_map<skr_async_io_request_t*, eastl::string> expression_paths;
        L2DRequestCallbackData* cbData;
    };
    class csmBreathVector : public csmVector<CubismBreath::BreathParameterData>
    {
    public:

    };
    class csmIdVector : public csmVector<CubismIdHandle>
    {
    public:

    };
}}}

#ifndef SKR_SERIALIZE_GURAD
void skr_live2d_model_create_from_json(skr_io_ram_service_t* ioService, const char* path, skr_live2d_ram_io_request_t* live2dRequest)
{
    ZoneScopedN("ioRAM Live2D Request");

    SKR_ASSERT(live2dRequest->vfs_override && "Support only vfs override");

    auto callbackData = SkrNew<L2DRequestCallbackData>();
    skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
    ramIO.path = path;
    ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
        auto cbData = (L2DRequestCallbackData*)data;
        auto model_resource = cbData->live2dRequest->model_resource = SkrNew<skr_live2d_model_resource_t>();
        auto model_setting = model_resource->model_setting 
            = SkrNew<L2DF::CubismModelSettingJson>(request->bytes, (L2DF::csmSizeInt)request->size);
        sakura_free(request->bytes);
        // setup models & expressions count
        cbData->model_count = model_setting->GetModelFileName() ? 1 : 0;
        cbData->expression_count = model_setting->GetExpressionCount();
        // load model & physics & pose & eyeblinks & breath & usrdata
        auto model = model_resource->model = SkrNew<L2DF::csmUserModel>();
        model->request(cbData->ioService, cbData);
        // load expressions
        auto expressions = model_resource->expression_map = SkrNew<L2DF::csmMotionMap>();
        expressions->request(cbData->ioService, cbData);
    };
    ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)callbackData;
    callbackData->live2dRequest = live2dRequest;
    callbackData->ioService = ioService;
    // TODO: replace this with newer VFS API
    std::string l2dHomePathStr;
    {
        ZoneScopedN("ioRAM Live2D Path Calc");
        auto l2dPath = ghc::filesystem::path(path);
        l2dHomePathStr = l2dPath.parent_path().u8string();
        callbackData->u8HomePath = l2dHomePathStr.c_str();
    }
    ioService->request(live2dRequest->vfs_override, &ramIO, &live2dRequest->settingsRequest);
}
#endif

void skr_live2d_model_free(skr_live2d_model_resource_id live2d_resource)
{
    SkrDelete(live2d_resource->model_setting);
    SkrDelete(live2d_resource->model);
    SkrDelete(live2d_resource->expression_map);
    SkrDelete(live2d_resource);
}