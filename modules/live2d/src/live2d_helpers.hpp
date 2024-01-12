#pragma once
#include "SkrLive2D/l2d_model_resource.h"
#include "Framework/CubismFramework.hpp" // IWYU pragma: export
#include "Framework/Type/csmMap.hpp" // IWYU pragma: export
#include "Framework/Effect/CubismBreath.hpp" // IWYU pragma: export
#include "Framework/Model/CubismUserModel.hpp" // IWYU pragma: export
#include "Framework/CubismModelSettingJson.hpp" // IWYU pragma: export
#include "Framework/Effect/CubismEyeBlink.hpp" // IWYU pragma: export
#include "Framework/Effect/CubismBreath.hpp" // IWYU pragma: export
#include "Framework/Motion/CubismMotion.hpp" // IWYU pragma: export
#include "Id/CubismIdManager.hpp" // IWYU pragma: export
#include "CubismDefaultParameterId.hpp" // IWYU pragma: export
#include "Utils/CubismString.hpp" // IWYU pragma: export

#include "SkrContainers/string.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrContainers/umap.hpp"
#include <float.h>

namespace L2DF = Live2D::Cubism::Framework;

struct IAsyncL2DResourceInterface {
    virtual void on_finished() = 0;
};

struct L2DRequestCallbackData
{
    skr_io_ram_service_t* ioService;
    IAsyncL2DResourceInterface* model_resource;
    IAsyncL2DResourceInterface* motions_resource;
    skr_live2d_ram_io_future_t* live2dRequest;   
    skr::String u8HomePath;

    skr::BlobId settingBlob;

    uint32_t expression_count;
    uint32_t motion_count;
    uint32_t model_count;
    uint32_t phys_count;
    uint32_t pose_count;
    uint32_t usr_data_count;
    SAtomicU32 finished_expressions;
    SAtomicU32 finished_motions;
    SAtomicU32 finished_models;
    SAtomicU32 finished_physics;
    SAtomicU32 finished_poses;
    SAtomicU32 finished_usr_data;

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

        skr::String homePath;

    protected:
        Csm::CubismMotionQueueEntryHandle startMotion(csmMotionMap* motion_map, const Csm::csmChar* group, Csm::csmInt32 no, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL) SKR_NOEXCEPT;
        Csm::CubismMotionQueueEntryHandle startRandomMotion(csmMotionMap* motion_map, const Csm::csmChar* group, Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler = NULL) SKR_NOEXCEPT;
    
        // Model States
        skr::stl_vector<uint32_t> _sorted_drawable_list;
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
        skr::String posePath;
        skr::String modelPath;
        skr::String pyhsicsPath;
        skr::String usrDataPath;
        skr_io_future_t poseFuture;
        skr::BlobId poseBlob;
        skr_io_future_t modelFuture;
        skr::BlobId modelBlob;
        skr_io_future_t pyhsicsFuture;
        skr::BlobId physicsBlob;
        skr_io_future_t usrDataFuture;
        skr::BlobId usrDataBlob;
        L2DRequestCallbackData* cbData;
    };
    class csmExpressionMap : public csmMap<csmString, ACubismMotion*>
    {
    public:
        ~csmExpressionMap() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;

        skr::stl_vector<skr_io_future_t> expressionFutures;
        skr::stl_vector<skr::BlobId> expressionBlobs;
        skr::UMap<skr_io_future_t*, skr::String> expressionNames;
        skr::UMap<skr_io_future_t*, skr::String> expressionPaths;
        L2DRequestCallbackData* cbData;
    };
    class csmMotionMap : public csmMap<csmString, csmVector<ACubismMotion*>>, public IAsyncL2DResourceInterface
    {
    public:
        ~csmMotionMap() SKR_NOEXCEPT;
        void request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT;
        void on_finished() SKR_NOEXCEPT final;

        skr::stl_vector<skr_io_future_t> motionFutures;
        skr::stl_vector<skr::BlobId> motionBlobs;
        skr::UMap<skr_io_future_t*, std::pair<skr::String, uint32_t>> motionEntries;
        skr::UMap<skr_io_future_t*, skr::String> motionPaths;
        L2DRequestCallbackData* cbData;
    };
}}}

#include "cgpu/flags.h"
#include "SkrRT/misc/types.h"
#include "Framework/Math/CubismMatrix44.hpp"
#include "Framework/Math/CubismViewMatrix.hpp"

struct live2d_render_view_t {
    Csm::CubismMatrix44 device_to_screen;
    Csm::CubismViewMatrix view_matrix;
    skr_float4_t clear_color;
};

const ECGPUFormat live2d_depth_format = CGPU_FORMAT_D32_SFLOAT;
const ECGPUFormat live2d_mask_format = CGPU_FORMAT_B8G8R8A8_UNORM;