#pragma once
#include "skr_live2d/model_resource.h"
#include "Framework/CubismFramework.hpp"
#include "Framework/Type/csmMap.hpp"
#include "Framework/Effect/CubismBreath.hpp"
#include "Framework/Model/CubismUserModel.hpp"
#include "Framework/CubismModelSettingJson.hpp"
#include "Framework/Effect/CubismEyeBlink.hpp"
#include "Framework/Effect/CubismBreath.hpp"
#include "Framework/Motion/CubismMotion.hpp"
#include "Id/CubismIdManager.hpp"
#include "CubismDefaultParameterId.hpp"
#include "Utils/CubismString.hpp"

#include <EASTL/string.h>
#include <EASTL/vector_map.h>

namespace L2DF = Live2D::Cubism::Framework;

struct IAsyncL2DResourceInterface {
    virtual void on_finished() = 0;
};

struct L2DRequestCallbackData
{
    skr_io_ram_service_t* ioService;
    IAsyncL2DResourceInterface* model_resource;
    IAsyncL2DResourceInterface* motions_resource;
    skr_live2d_ram_io_request_t* live2dRequest;   
    eastl::string u8HomePath;

    uint32_t expression_count;
    uint32_t motion_count;
    uint32_t model_count;
    uint32_t phys_count;
    uint32_t pose_count;
    uint32_t usr_data_count;
    SAtomic32 finished_expressions;
    SAtomic32 finished_motions;
    SAtomic32 finished_models;
    SAtomic32 finished_physics;
    SAtomic32 finished_poses;
    SAtomic32 finished_usr_data;

    void partial_finished() SKR_NOEXCEPT;
};

namespace Live2D { namespace Cubism { namespace Framework {
    class csmUserModel : public CubismUserModel, public IAsyncL2DResourceInterface
    {
        friend class csmMotionMap;
    public:
        csmUserModel() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;
        void on_finished() SKR_NOEXCEPT final;

    protected:
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
    class csmExpressionMap : public csmMap<csmString, ACubismMotion*>
    {
    public:
        ~csmExpressionMap() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;

        eastl::vector<skr_async_io_request_t> expression_requests;
        eastl::vector_map<skr_async_io_request_t*, eastl::string> expression_names;
        eastl::vector_map<skr_async_io_request_t*, eastl::string> expression_paths;
        L2DRequestCallbackData* cbData;
    };
    class csmMotionMap : public csmMap<csmString, csmVector<ACubismMotion*>>, public IAsyncL2DResourceInterface
    {
    public:
        ~csmMotionMap() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;
        void on_finished() SKR_NOEXCEPT final;

        eastl::vector<skr_async_io_request_t> motion_requests;
        eastl::vector_map<skr_async_io_request_t*, eastl::pair<eastl::string, uint32_t>> motion_entries;
        eastl::vector_map<skr_async_io_request_t*, eastl::string> motion_paths;
        L2DRequestCallbackData* cbData;
    };

}}}