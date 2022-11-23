#pragma once
#include "SkrLive2D/l2d_model_resource.h"
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

#include <containers/string.hpp>
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
    skr::string u8HomePath;

    skr_async_ram_destination_t settingRawData;

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
    static const csmChar* kMotionGroupIdle = "Idle"; // アイドリング
    static const csmChar* kMotionGroupTapBody = "TapBody"; // 体をタップしたとき
    // モーションの優先度定数
    static constexpr csmInt32 kPriorityNone = 0;
    static constexpr csmInt32 kPriorityIdle = 1;
    static constexpr csmInt32 kPriorityNormal = 2;
    static constexpr csmInt32 kPriorityForce = 3;
    static constexpr csmInt32 kColorChannelCount = 4;
    // TODO: Refactor these
    static constexpr csmInt32 kMaskResolution = 256;

    class csmUserModel : public CubismUserModel, public IAsyncL2DResourceInterface
    {
        friend class csmMotionMap;
    public:
        csmUserModel() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;
        void on_finished() SKR_NOEXCEPT final;

        void update(csmMotionMap* motion_map, float delta_time) SKR_NOEXCEPT;
        const uint32_t* get_sorted_drawlist() const SKR_NOEXCEPT;

        skr::string homePath;

    protected:
        Csm::CubismMotionQueueEntryHandle startMotion(csmMotionMap* motion_map, const Csm::csmChar* group, Csm::csmInt32 no, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL) SKR_NOEXCEPT;
        Csm::CubismMotionQueueEntryHandle startRandomMotion(csmMotionMap* motion_map, const Csm::csmChar* group, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL) SKR_NOEXCEPT;
    
        // Model States
        eastl::vector<uint32_t> _sorted_drawable_list;
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
        skr::string posePath;
        skr::string modelPath;
        skr::string pyhsicsPath;
        skr::string usrDataPath;
        skr_async_request_t poseRequest;
        skr_async_ram_destination_t poseDestination;
        skr_async_request_t modelRequest;
        skr_async_ram_destination_t modelDestination;
        skr_async_request_t pyhsicsRequest;
        skr_async_ram_destination_t physicsDestination;
        skr_async_request_t usrDataRequest;
        skr_async_ram_destination_t usrDataDestination;
        L2DRequestCallbackData* cbData;
    };
    class csmExpressionMap : public csmMap<csmString, ACubismMotion*>
    {
    public:
        ~csmExpressionMap() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;

        eastl::vector<skr_async_request_t> expressionRequests;
        eastl::vector<skr_async_ram_destination_t> expressionDestinations;
        eastl::vector_map<skr_async_request_t*, skr::string> expressionNames;
        eastl::vector_map<skr_async_request_t*, skr::string> expressionPaths;
        L2DRequestCallbackData* cbData;
    };
    class csmMotionMap : public csmMap<csmString, csmVector<ACubismMotion*>>, public IAsyncL2DResourceInterface
    {
    public:
        ~csmMotionMap() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;
        void on_finished() SKR_NOEXCEPT final;

        eastl::vector<skr_async_request_t> motionRequests;
        eastl::vector<skr_async_ram_destination_t> motionDestinations;
        eastl::vector_map<skr_async_request_t*, eastl::pair<skr::string, uint32_t>> motionEntries;
        eastl::vector_map<skr_async_request_t*, skr::string> motionPaths;
        L2DRequestCallbackData* cbData;
    };
}}}

#include "cgpu/flags.h"
#include "utils/types.h"
#include "Framework/Math/CubismMatrix44.hpp"
#include "Framework/Math/CubismViewMatrix.hpp"

struct live2d_render_view_t {
    Csm::CubismMatrix44 device_to_screen;
    Csm::CubismViewMatrix view_matrix;
    skr_float4_t clear_color;
};

const ECGPUFormat live2d_depth_format = CGPU_FORMAT_D32_SFLOAT;
const ECGPUFormat live2d_mask_format = CGPU_FORMAT_B8G8R8A8_UNORM;