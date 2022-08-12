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
#include "Framework/Effect/CubismEyeBlink.hpp"
#include "Framework/Effect/CubismBreath.hpp"
#include "Id/CubismIdManager.hpp"
#include "CubismDefaultParameterId.hpp"

#include "tracy/Tracy.hpp"

struct IAsyncL2DResourceInterface {
    virtual void on_finished() = 0;
};

struct L2DRequestCallbackData
{
    skr_io_ram_service_t* ioService;
    IAsyncL2DResourceInterface* model_resource;
    skr_live2d_ram_io_request_t* live2dRequest;   
    eastl::string u8HomePath;

    uint32_t expression_count;
    uint32_t model_count;
    uint32_t phys_count;
    uint32_t pose_count;
    uint32_t usr_data_count;
    SAtomic32 finished_expressions;
    SAtomic32 finished_models;
    SAtomic32 finished_physics;
    SAtomic32 finished_poses;
    SAtomic32 finished_usr_data;

    void partial_finished()
    {
        auto _e = skr_atomic32_load_acquire(&finished_expressions);
        auto _m = skr_atomic32_load_acquire(&finished_models);
        auto _ph = skr_atomic32_load_acquire(&finished_physics);
        auto _po = skr_atomic32_load_acquire(&finished_poses);
        auto _ud = skr_atomic32_load_acquire(&finished_usr_data);
        if (_e == expression_count && _m == model_count && _ph == phys_count && _po == pose_count && _ud == usr_data_count)
        {
            model_resource->on_finished();
            skr_atomic32_store_relaxed(&live2dRequest->liv2d_status, SKR_ASYNC_IO_STATUS_OK);
            SkrDelete(this);
        }
    }
};

namespace L2DF = Live2D::Cubism::Framework;

namespace Live2D { namespace Cubism { namespace Framework {
    class csmUserModel : public CubismUserModel, public IAsyncL2DResourceInterface
    {
    public:
        csmUserModel()
            : CubismUserModel()
        {
            _idParamAngleX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleX);
            _idParamAngleY = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleY);
            _idParamAngleZ = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleZ);
            _idParamBodyAngleX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamBodyAngleX);
            _idParamEyeBallX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamEyeBallX);
            _idParamEyeBallY = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamEyeBallY);
        }

        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data)
        {
            ZoneScopedN("Live2D Model Request");

            auto settings = _modelSetting = data->live2dRequest->model_resource->model_setting;
            auto mFile = settings->GetModelFileName();
            auto phFile = settings->GetPhysicsFileName();
            auto poFile = settings->GetPoseFileName();
            auto udFile = settings->GetUserDataFile();

            cbData = data;
            cbData->model_resource = this;

            SKR_LOG_TRACE("Read Live2D From Home %s", data->u8HomePath.c_str());

            // Model Request
            {
                ZoneScopedN("Model Request");

                modelPath = data->u8HomePath + "/" + mFile;
                SKR_LOG_TRACE("Live2D Model %s at %s", mFile, modelPath.c_str());
                skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
                ramIO.path = modelPath.c_str();
                ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                    ZoneScopedN("Model Initialize");
                    auto _this = (csmUserModel*)data;

                    _this->LoadModel(request->bytes, (L2DF::csmSizeInt)request->size);
                    sakura_free(request->bytes);
                    
                    skr_atomic32_add_relaxed(&_this->cbData->finished_models, 1);
                    _this->cbData->partial_finished();
                };
                ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)this;
                ioService->request(cbData->live2dRequest->vfs_override, &ramIO, &modelRequest);
            }
            // Physics Request
            if (cbData->phys_count)
            {
                ZoneScopedN("Physics Request");

                pyhsicsPath = data->u8HomePath + "/" + phFile;
                SKR_LOG_TRACE("Live2D Physics %s at %s", phFile, pyhsicsPath.c_str());
                skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
                ramIO.path = pyhsicsPath.c_str();
                ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                    ZoneScopedN("Physics Initialize");
                    auto _this = (csmUserModel*)data;
                    
                    _this->LoadPhysics(request->bytes, (L2DF::csmSizeInt)request->size);
                    sakura_free(request->bytes);
                    
                    skr_atomic32_add_relaxed(&_this->cbData->finished_physics, 1);
                    _this->cbData->partial_finished();
                };
                ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)this;
                ioService->request(cbData->live2dRequest->vfs_override, &ramIO, &pyhsicsRequest);
            }
            // Pose Request
            if (cbData->pose_count)
            {
                ZoneScopedN("Pose Request");

                posePath = data->u8HomePath + "/" + poFile;
                SKR_LOG_TRACE("Live2D Pose %s at %s", poFile, posePath.c_str());
                skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
                ramIO.path = posePath.c_str();
                ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                    ZoneScopedN("Pose Initialize");
                    auto _this = (csmUserModel*)data;
                    
                    _this->LoadPose(request->bytes, (L2DF::csmSizeInt)request->size);
                    sakura_free(request->bytes);
                    
                    skr_atomic32_add_relaxed(&_this->cbData->finished_poses, 1);
                    _this->cbData->partial_finished();
                };
                ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)this;
                ioService->request(cbData->live2dRequest->vfs_override, &ramIO, &poseRequest);
            }
            // UsrData Request
            if (cbData->usr_data_count)
            {
                ZoneScopedN("UsrData Request");

                usrDataPath = data->u8HomePath + "/" + udFile;
                SKR_LOG_TRACE("Live2D UsrData %s at %s", udFile, usrDataPath.c_str());
                skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
                ramIO.path = usrDataPath.c_str();
                ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                    ZoneScopedN("UsrData Initialize");
                    auto _this = (csmUserModel*)data;
                    
                    _this->LoadUserData(request->bytes, (L2DF::csmSizeInt)request->size);
                    sakura_free(request->bytes);
                    
                    skr_atomic32_add_relaxed(&_this->cbData->finished_usr_data, 1);
                    _this->cbData->partial_finished();
                };
                ramIO.callback_datas[SKR_ASYNC_IO_STATUS_OK] = (void*)this;
                ioService->request(cbData->live2dRequest->vfs_override, &ramIO, &usrDataRequest);
            }
        }

        void PreloadMotionGroup(const Csm::csmChar* group)
        {

        }

        void on_finished() override
        {
            ZoneScopedN("Live2D Model Finish");

            // EyeBlink Parameters
            if (_modelSetting->GetEyeBlinkParameterCount())
            {
                _eyeBlink = CubismEyeBlink::Create(_modelSetting);
            }
            // Breath Paramters
            {
                _breath = CubismBreath::Create();
                csmVector<CubismBreath::BreathParameterData> breathParameters;

                breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleX, 0.0f, 15.0f, 6.5345f, 0.5f));
                breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleY, 0.0f, 8.0f, 3.5345f, 0.5f));
                breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamAngleZ, 0.0f, 10.0f, 5.5345f, 0.5f));
                breathParameters.PushBack(CubismBreath::BreathParameterData(_idParamBodyAngleX, 0.0f, 4.0f, 15.5345f, 0.5f));
                breathParameters.PushBack(CubismBreath::BreathParameterData(CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamBreath), 0.5f, 0.5f, 3.2345f, 0.5f));

                _breath->SetParameters(breathParameters);
            }
            // EyeBlinkIds
            {
                csmInt32 eyeBlinkIdCount = _modelSetting->GetEyeBlinkParameterCount();
                for (csmInt32 i = 0; i < eyeBlinkIdCount; ++i)
                {
                    _eyeBlinkIds.PushBack(_modelSetting->GetEyeBlinkParameterId(i));
                }
            }
            // LipSyncIds
            {
                csmInt32 lipSyncIdCount = _modelSetting->GetLipSyncParameterCount();
                for (csmInt32 i = 0; i < lipSyncIdCount; ++i)
                {
                    _lipSyncIds.PushBack(_modelSetting->GetLipSyncParameterId(i));
                }
            }
            //Layout
            csmMap<csmString, csmFloat32> layout;
            _modelSetting->GetLayoutMap(layout);
            _modelMatrix->SetupFromLayout(layout);

            _model->SaveParameters();

            for (csmInt32 i = 0; i < _modelSetting->GetMotionGroupCount(); i++)
            {
                const csmChar* group = _modelSetting->GetMotionGroupName(i);
                PreloadMotionGroup(group);
            }

            _motionManager->StopAllMotions();
        }

        // Model States
        Csm::ICubismModelSetting* _modelSetting;
        Csm::csmVector<Csm::CubismIdHandle> _eyeBlinkIds;
        Csm::csmVector<Csm::CubismIdHandle> _lipSyncIds; ///< モデルに設定されたリップシンク機能用パラメータID
        const Csm::CubismId* _idParamAngleX; ///< パラメータID: ParamAngleX
        const Csm::CubismId* _idParamAngleY; ///< パラメータID: ParamAngleX
        const Csm::CubismId* _idParamAngleZ; ///< パラメータID: ParamAngleX
        const Csm::CubismId* _idParamBodyAngleX; ///< パラメータID: ParamBodyAngleX
        const Csm::CubismId* _idParamEyeBallX; ///< パラメータID: ParamEyeBallX
        const Csm::CubismId* _idParamEyeBallY; ///< パラメータID: ParamEyeBallXY

        // Async Requests
        eastl::string posePath;
        eastl::string modelPath;
        eastl::string pyhsicsPath;
        eastl::string usrDataPath;
        skr_async_io_request_t poseRequest;
        skr_async_io_request_t modelRequest;
        skr_async_io_request_t pyhsicsRequest;
        skr_async_io_request_t usrDataRequest;
        L2DRequestCallbackData* cbData;
    };
    class csmMotionMap : public csmMap<csmString, ACubismMotion*>
    {
    public:
        ~csmMotionMap()
        {
            for (auto iter = Begin(); iter != End(); ++iter)
            {
                ACubismMotion::Delete(iter->Second);
            }
            Clear();
        }
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data)
        {
            ZoneScopedN("Live2D MotionMap Request");
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

                SKR_LOG_TRACE("Request Live2D Expression %s at %s", name.c_str(), file);

                skr_ram_io_t ramIO = make_zeroed<skr_ram_io_t>();
                ramIO.path = path.c_str();
                ramIO.callbacks[SKR_ASYNC_IO_STATUS_OK] = +[](skr_async_io_request_t* request, void* data) noexcept {
                    auto _this = (csmMotionMap*)data;
                    auto name = _this->expression_names[request];
                    
                    csmMap<csmString, ACubismMotion*>& map = *_this;
                    map[name.c_str()] = CubismExpressionMotion::Create(request->bytes, (L2DF::csmSizeInt)request->size);
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
        ZoneScopedN("Live2D Request By Settings");

        auto cbData = (L2DRequestCallbackData*)data;
        auto model_resource = cbData->live2dRequest->model_resource = SkrNew<skr_live2d_model_resource_t>();
        auto model_setting = model_resource->model_setting 
            = SkrNew<L2DF::CubismModelSettingJson>(request->bytes, (L2DF::csmSizeInt)request->size);
        sakura_free(request->bytes);
        // setup models & expressions count
        if (auto _ = eastl::string(model_setting->GetModelFileName()); !_.empty())
        {
            cbData->model_count = 1;
        }
        if (auto _ = eastl::string(model_setting->GetPhysicsFileName()); !_.empty())
        {
            cbData->phys_count = 1;
        }
        if (auto _ = eastl::string(model_setting->GetPoseFileName()); !_.empty())
        {
            cbData->pose_count = 1;
        }
        if (auto _ = eastl::string(model_setting->GetUserDataFile()); !_.empty())
        {
            cbData->usr_data_count = 1;
        }
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
    SkrDelete(live2d_resource->motion_map);
    SkrDelete(live2d_resource);
}