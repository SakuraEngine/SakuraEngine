#include <SkrOS/filesystem.hpp>

#include "SkrBase/misc/debug.h" 
#include "SkrCore/memory/memory.h"
#include "SkrCore/log.h"
#include "SkrRT/io/ram_io.hpp"
#include "SkrCore/platform/vfs.h"

#include "live2d_helpers.hpp"

#include "SkrProfile/profile.h"

void L2DRequestCallbackData::partial_finished() SKR_NOEXCEPT
{
    auto _e = skr_atomic_load_acquire(&finished_expressions);
    auto _m = skr_atomic_load_acquire(&finished_models);
    auto _ph = skr_atomic_load_acquire(&finished_physics);
    auto _po = skr_atomic_load_acquire(&finished_poses);
    auto _ud = skr_atomic_load_acquire(&finished_usr_data);
    auto _mo = skr_atomic_load_acquire(&finished_motions);
    if (_e == expression_count && _m == model_count && _ph == phys_count &&
            _po == pose_count && _ud == usr_data_count && _mo == motion_count)
    {
        model_resource->on_finished();
        motions_resource->on_finished();
        skr_atomic_store_relaxed(&live2dRequest->liv2d_status, SKR_IO_STAGE_COMPLETED);
        live2dRequest->finish_callback(live2dRequest, live2dRequest->callback_data);
        SkrDelete(this);
    }
}

namespace Live2D { namespace Cubism { namespace Framework {

csmUserModel::csmUserModel() SKR_NOEXCEPT
    : CubismUserModel()
{
    _idParamAngleX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleX);
    _idParamAngleY = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleY);
    _idParamAngleZ = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamAngleZ);
    _idParamBodyAngleX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamBodyAngleX);
    _idParamEyeBallX = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamEyeBallX);
    _idParamEyeBallY = CubismFramework::GetIdManager()->GetId(DefaultParameterId::ParamEyeBallY);
}

void csmUserModel::request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT
{
    SkrZoneScopedN("Request Live2D Model");

    auto settings = _modelSetting = data->live2dRequest->model_resource->model_setting;
    auto mFile = settings->GetModelFileName();
    auto phFile = settings->GetPhysicsFileName();
    auto poFile = settings->GetPoseFileName();
    auto udFile = settings->GetUserDataFile();

    homePath = data->u8HomePath;
    cbData = data;
    cbData->model_resource = this;

    SKR_LOG_TRACE(u8"Read Live2D From Home %s", data->u8HomePath.c_str());
    auto batch = ioService->open_batch(4); // 4 file requests
    // Model Request
    {
        SkrZoneScopedN("Request Model");

        modelPath.append(data->u8HomePath);
        modelPath.append(u8"/");
        modelPath.append((const char8_t*)mFile);

        SKR_LOG_TRACE(u8"Live2D Model %s at %s", mFile, modelPath.c_str());
        auto rq = ioService->open_request();
        rq->set_vfs(cbData->live2dRequest->vfs_override);
        rq->set_path(modelPath.c_str());
        rq->set_path(modelPath.c_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED,
        +[](skr_io_future_t* future, skr_io_request_t* request, void* usrdata) noexcept {
            SkrZoneScopedN("Create Model");
            auto _this = (csmUserModel*)usrdata;
            auto& blob = _this->modelBlob;

            _this->LoadModel(blob->get_data(), (L2DF::csmSizeInt)blob->get_size());
            blob.reset();
            
            skr_atomic_fetch_add_relaxed(&_this->cbData->finished_models, 1);
            _this->cbData->partial_finished();
        }, this);
        rq->use_async_complete();
        // modelBlob = ioService->request(rq, &modelFuture);
        modelBlob = skr::static_pointer_cast<skr::io::IRAMIOBuffer>(batch->add_request(rq, &modelFuture));
    }
    // Physics Request
    if (cbData->phys_count)
    {
        SkrZoneScopedN("Request Physics");

        pyhsicsPath.append(data->u8HomePath);
        pyhsicsPath.append(u8"/");
        pyhsicsPath.append((const char8_t*)phFile);

        SKR_LOG_TRACE(u8"Live2D Physics %s at %s", phFile, pyhsicsPath.c_str());
        auto rq = ioService->open_request();
        rq->set_vfs(cbData->live2dRequest->vfs_override);
        rq->set_path(pyhsicsPath.c_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED,
        +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
            SkrZoneScopedN("Create Physics");
            auto _this = (csmUserModel*)data;
            auto& blob = _this->physicsBlob;
            
            _this->LoadPhysics(blob->get_data(), (L2DF::csmSizeInt)blob->get_size());
            blob.reset();
            
            skr_atomic_fetch_add_relaxed(&_this->cbData->finished_physics, 1);
            _this->cbData->partial_finished();
        }, this);
        rq->use_async_complete();
        // physicsBlob = ioService->request(rq, &pyhsicsFuture);
        physicsBlob = skr::static_pointer_cast<skr::io::IRAMIOBuffer>(batch->add_request(rq, &pyhsicsFuture));
    }
    // Pose Request
    if (cbData->pose_count)
    {
        SkrZoneScopedN("Request Pose");

        posePath.append(data->u8HomePath);
        posePath.append(u8"/");
        posePath.append((const char8_t*)poFile);

        SKR_LOG_TRACE(u8"Live2D Pose %s at %s", poFile, posePath.c_str());
        auto rq = ioService->open_request();
        rq->set_vfs(cbData->live2dRequest->vfs_override);
        rq->set_path(posePath.c_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED,
        +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
            SkrZoneScopedN("Create Pose");
            auto _this = (csmUserModel*)data;
            auto& blob = _this->poseBlob;
            
            _this->LoadPose(blob->get_data(), (L2DF::csmSizeInt)blob->get_size());
            blob.reset();
            
            skr_atomic_fetch_add_relaxed(&_this->cbData->finished_poses, 1);
            _this->cbData->partial_finished();
        }, this);
        rq->use_async_complete();
        // poseBlob = ioService->request(rq, &poseFuture);
        poseBlob = skr::static_pointer_cast<skr::io::IRAMIOBuffer>(batch->add_request(rq, &poseFuture));
    }
    // UsrData Request
    if (cbData->usr_data_count)
    {
        SkrZoneScopedN("Request UsrData");

        usrDataPath = data->u8HomePath;
        usrDataPath.append(u8"/");
        usrDataPath.append((const char8_t*)udFile);
        
        SKR_LOG_TRACE(u8"Live2D UsrData %s at %s", udFile, usrDataPath.c_str());
        auto rq = ioService->open_request();
        rq->set_vfs(cbData->live2dRequest->vfs_override);
        rq->set_path(usrDataPath.c_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED,
        +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
            SkrZoneScopedN("Create UsrData");
            auto _this = (csmUserModel*)data;
            auto& blob = _this->usrDataBlob;
            
            _this->LoadUserData(blob->get_data(), (L2DF::csmSizeInt)blob->get_size());
            blob.reset();
            
            skr_atomic_fetch_add_relaxed(&_this->cbData->finished_usr_data, 1);
            _this->cbData->partial_finished();
        }, this);
        rq->use_async_complete();
        // usrDataBlob = ioService->request(rq, &usrDataFuture);
        usrDataBlob = skr::static_pointer_cast<skr::io::IRAMIOBuffer>(batch->add_request(rq, &usrDataFuture));
    }
    ioService->request(batch);
}

void csmUserModel::on_finished() SKR_NOEXCEPT
{
    SkrZoneScopedN("Setup Live2D Model");

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
}

CubismMotionQueueEntryHandle csmUserModel::startMotion(csmMotionMap* motion_map, const csmChar* group, 
    csmInt32 no, csmInt32 priority, ACubismMotion::FinishedMotionCallback onFinishedMotionHandler) SKR_NOEXCEPT
{
    if (priority == kPriorityForce)
    {
        _motionManager->SetReservePriority(priority);
    }
    else if (!_motionManager->ReserveMotion(priority))
    {
        if (_debugMode)
        {
            SKR_LOG_TRACE(u8"[csmUserModel]can't start motion.");
        }
        return InvalidMotionQueueEntryHandleValue;
    }

    const csmString fileName = _modelSetting->GetMotionFileName(group, no);

    //ex) idle_0
    auto& map = *static_cast< csmMap<csmString, csmVector<ACubismMotion*>>* >(motion_map);
    CubismMotion* motion = static_cast<CubismMotion*>(map[group][no]);
    csmBool autoDelete = false;
    SKR_ASSERT(motion && "motion is null");
    motion->SetFinishedMotionHandler(onFinishedMotionHandler);
    //voice
    /*
    csmString voice = _modelSetting->GetMotionSoundFileName(group, no);
    if (strcmp(voice.GetRawString(), "") != 0)
    {
        csmString path = voice;
        path = _modelHomeDir + path;
        _wavFileHandler.Start(path);
    }
    */
    if (_debugMode)
    {
        SKR_LOG_TRACE(u8"[APP]start motion: [%s_%d]", group, no);
    }
    return  _motionManager->StartMotionPriority(motion, autoDelete, priority);
}

Csm::CubismMotionQueueEntryHandle csmUserModel::startRandomMotion(csmMotionMap* motion_map, const Csm::csmChar* group, 
    Csm::csmInt32 priority, Csm::ACubismMotion::FinishedMotionCallback onFinishedMotionHandler) SKR_NOEXCEPT
{
    if (_modelSetting->GetMotionCount(group) == 0)
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    csmInt32 no = rand() % _modelSetting->GetMotionCount(group);

    return startMotion(motion_map, group, no, priority, onFinishedMotionHandler);
}

void csmUserModel::update(csmMotionMap* motion_map, float delta_time) SKR_NOEXCEPT
{
    _dragManager->Update(delta_time);
    _dragX = _dragManager->GetX();
    _dragY = _dragManager->GetY();

    // モーションによるパラメータ更新の有無
    csmBool motionUpdated = false;

    //-----------------------------------------------------------------
    _model->LoadParameters(); // 前回セーブされた状態をロード
    if (_motionManager->IsFinished())
    {
        // モーションの再生がない場合、待機モーションの中からランダムで再生する
        startRandomMotion(motion_map, kMotionGroupIdle, kPriorityIdle);
    }
    else
    {
        motionUpdated = _motionManager->UpdateMotion(_model, delta_time); // モーションを更新
    }
    _model->SaveParameters(); // 状態を保存
    //-----------------------------------------------------------------

    // まばたき
    if (!motionUpdated)
    {
        if (_eyeBlink != NULL)
        {
            // メインモーションの更新がないとき
            _eyeBlink->UpdateParameters(_model, delta_time); // 目パチ
        }
    }

    if (_expressionManager != NULL)
    {
        _expressionManager->UpdateMotion(_model, delta_time); // 表情でパラメータ更新（相対変化）
    }

    //ドラッグによる変化
    //ドラッグによる顔の向きの調整
    _model->AddParameterValue(_idParamAngleX, _dragX * 30); // -30から30の値を加える
    _model->AddParameterValue(_idParamAngleY, _dragY * 30);
    _model->AddParameterValue(_idParamAngleZ, _dragX * _dragY * -30);

    //ドラッグによる体の向きの調整
    _model->AddParameterValue(_idParamBodyAngleX, _dragX * 10); // -10から10の値を加える

    //ドラッグによる目の向きの調整
    _model->AddParameterValue(_idParamEyeBallX, _dragX); // -1から1の値を加える
    _model->AddParameterValue(_idParamEyeBallY, _dragY);

    // 呼吸など
    if (_breath != NULL)
    {
        _breath->UpdateParameters(_model, delta_time);
    }

    // 物理演算の設定
    if (_physics != NULL)
    {
        _physics->Evaluate(_model, delta_time);
    }

    // リップシンクの設定
    /*
    if (_lipSync)
    {
        // リアルタイムでリップシンクを行う場合、システムから音量を取得して0〜1の範囲で値を入力します。
        csmFloat32 value = 0.0f;

        // 状態更新/RMS値取得
        _wavFileHandler.Update(delta_time);
        value = _wavFileHandler.GetRms();

        for (csmUint32 i = 0; i < _lipSyncIds.GetSize(); ++i)
        {
            _model->AddParameterValue(_lipSyncIds[i], value, 0.8f);
        }
    }
    */

    // ポーズの設定
    if (_pose != NULL)
    {
        _pose->UpdateParameters(_model, delta_time);
    }

    _model->Update();
    
    const auto* orders = _model->GetDrawableRenderOrders();
    _sorted_drawable_list.resize(GetModel()->GetDrawableCount());
    for (uint32_t i = 0; i < _sorted_drawable_list.size(); i++)
    {
        const auto order = orders[i];
        _sorted_drawable_list[order] = i;
    }
}

const uint32_t* csmUserModel::get_sorted_drawlist() const SKR_NOEXCEPT
{
    return _sorted_drawable_list.data();
}

csmExpressionMap::~csmExpressionMap() SKR_NOEXCEPT
{
    for (auto iter = Begin(); iter != End(); ++iter)
    {
        ACubismMotion::Delete(iter->Second);
    }
    Clear();
}

void csmExpressionMap::request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT
{
    SkrZoneScopedN("Request Live2D ExpressionMap");
    cbData = data;

    auto settings = data->live2dRequest->model_resource->model_setting;
    expressionNames.reserve(settings->GetExpressionCount());
    expressionPaths.reserve(settings->GetExpressionCount());
    expressionFutures.resize(settings->GetExpressionCount());
    expressionBlobs.resize(settings->GetExpressionCount());
    auto batch = ioService->open_batch(expressionFutures.size());
    for (uint32_t i = 0; i < expressionFutures.size(); i++)
    {
        auto& future = expressionFutures[i];

        auto name = (const char8_t*)settings->GetExpressionName(i);
        auto file = settings->GetExpressionFileName(i);

        auto path = data->u8HomePath;
        path.append(u8"/");
        path.append((const char8_t*)file);
        expressionPaths.add(&future, path);
        expressionNames.add(&future, name);

        SKR_LOG_TRACE(u8"Request Live2D Expression %s at %s", name, file);

        auto rq = ioService->open_request();
        rq->set_vfs(cbData->live2dRequest->vfs_override);
        rq->set_path(path.c_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED,
        +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
            SkrZoneScopedN("Create Live2D Expression");

            auto _this = (csmExpressionMap*)data;
            const auto& name = _this->expressionNames.find(future).value();
            auto index = future - _this->expressionFutures.data();
            auto& blob = _this->expressionBlobs[index];
            
            csmMap<csmString, ACubismMotion*>& map = *_this;
            // callbacks are called from single same thread, so no need to lock 
            map[name.c_str_raw()] = CubismExpressionMotion::Create(blob->get_data(), (L2DF::csmSizeInt)blob->get_size());
            blob.reset();
            
            skr_atomic_fetch_add_relaxed(&_this->cbData->finished_expressions, 1);
            _this->cbData->partial_finished();
        }, this);
        rq->use_async_complete();
        // expressionBlobs[i] = ioService->request(rq, &future);
        expressionBlobs[i] = skr::static_pointer_cast<skr::io::IRAMIOBuffer>(batch->add_request(rq, &future));
    }
    ioService->request(batch);
}

csmMotionMap::~csmMotionMap() SKR_NOEXCEPT
{
    for (auto iter = Begin(); iter != End(); ++iter)
    {
        for (auto iter2 = iter->Second.Begin(); iter2 != iter->Second.End(); ++iter2)
        {
            ACubismMotion::Delete(*iter2);
        }
    }
    Clear();
}
void csmMotionMap::request(skr_io_ram_service_t* ioService, L2DRequestCallbackData* data) SKR_NOEXCEPT
{
    SkrZoneScopedN("Request Live2D MotionMap");
    cbData = data;
    cbData->motions_resource = this;

    auto settings = data->live2dRequest->model_resource->model_setting;
    motionEntries.reserve(cbData->motion_count);
    motionPaths.reserve(cbData->motion_count);
    motionFutures.resize(cbData->motion_count);
    motionBlobs.resize(cbData->motion_count);
    uint32_t slot = 0;
    for (uint32_t i = 0; i < (uint32_t)settings->GetMotionGroupCount(); i++)
    {
        SkrZoneScopedN("Setup Motion Request");

        auto group = settings->GetMotionGroupName(i);
        auto count = settings->GetMotionCount(group);
        csmMap<csmString, csmVector<ACubismMotion*>>& map = *this;
        map[group].Resize(count);
        for (uint32_t j = 0; j < (uint32_t)settings->GetMotionCount(group); j++)
        {
            auto& request = motionFutures[slot];
            std::pair<skr::String, uint32_t> entry = {(const char8_t*)group, j };
            auto file = settings->GetMotionFileName(group, j);
            auto path = skr::format(u8"{}/{}", data->u8HomePath, (const char8_t*)file);

            motionPaths.add(&request, path);
            motionEntries.add(&request, entry);
            slot++;
        }
    }
    auto batch = ioService->open_batch(motionFutures.size());
    SKR_ASSERT(motionPaths.is_compact() && "use at() API in an sparse map is not safe");
    SKR_ASSERT(motionEntries.is_compact() && "use at() API in an sparse map is not safe");
    for (uint32_t i = 0; i < motionFutures.size(); i++)
    {
        SkrZoneScopedN("Request Motion");

        auto& future = motionFutures[i];
        const auto& path = motionPaths.at(i).value;
        const auto& entry = motionEntries.at(i).value;
        SKR_LOG_TRACE(u8"Request Live2D Motion in group %s at %d", entry.first.c_str(), entry.second);

        auto rq = ioService->open_request();
        rq->set_vfs(cbData->live2dRequest->vfs_override);
        rq->set_path(path.c_str());
        rq->add_block({}); // read all
        rq->add_callback(SKR_IO_STAGE_COMPLETED,
        +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
            SkrZoneScopedN("Create Live2D Motion");

            auto _this = (csmMotionMap*)data;
            const auto& entry = _this->motionEntries.find(future).value();
            auto index = future - _this->motionFutures.data();
            auto& blob = _this->motionBlobs[index];
            
            csmMap<csmString, csmVector<ACubismMotion*>>& map = *_this;
            // callbacks are called from single same thread, so no need to lock 
            map[entry.first.c_str_raw()][entry.second] = L2DF::CubismMotion::Create(blob->get_data(), (L2DF::csmSizeInt)blob->get_size());
            blob.reset();

            skr_atomic_fetch_add_relaxed(&_this->cbData->finished_motions, 1);
            _this->cbData->partial_finished();
        }, this);
        rq->use_async_complete();
        // motionBlobs[i] = ioService->request(rq, &future);
        motionBlobs[i] = skr::static_pointer_cast<skr::io::IRAMIOBuffer>(batch->add_request(rq, &future));
    }
    ioService->request(batch);
}

void csmMotionMap::on_finished() SKR_NOEXCEPT
{
    SkrZoneScopedN("Setup Live2D Motion");

    auto Model = static_cast<csmUserModel*>(cbData->model_resource);
    auto settings = Model->_modelSetting;
    auto motionManager = Model->_motionManager;
    for (csmInt32 i = 0; i < settings->GetMotionGroupCount(); i++)
    {
        const csmChar* group = settings->GetMotionGroupName(i);
        for (uint32_t j = 0; j < (uint32_t)settings->GetMotionCount(group); j++)
        {
            SKR_LOG_TRACE(u8"Setup Live2D Motion %s at %d", group, j);
            csmMap<csmString, csmVector<ACubismMotion*>>& map = *this;
            auto motion = static_cast<CubismMotion*>(map[group][j]);
            csmFloat32 fadeTime = settings->GetMotionFadeInTimeValue(group, j);
            if (fadeTime >= 0.0f)
            {
                motion->SetFadeInTime(fadeTime);
            }
            motion->SetEffectIds(Model->_eyeBlinkIds, Model->_lipSyncIds);
        }
    }
    motionManager->StopAllMotions();
}

}}}

#ifndef SKR_SERIALIZE_GURAD
void skr_live2d_model_create_from_json(skr_io_ram_service_t* ioService, const char8_t* path, skr_live2d_ram_io_future_t* live2dRequest)
{
    SkrZoneScopedN("ioRAM Live2D Request");

    SKR_ASSERT(live2dRequest->vfs_override && "Support only vfs override");

    auto callbackData = SkrNew<L2DRequestCallbackData>();
    auto rq = ioService->open_request();
    rq->set_vfs(live2dRequest->vfs_override);
    rq->set_path(path);
    rq->add_block({}); // read all
    rq->add_callback(SKR_IO_STAGE_COMPLETED,
    +[](skr_io_future_t* future, skr_io_request_t* request, void* data) noexcept {
        SkrZoneScopedN("Live2D Request By Settings");

        auto cbData = (L2DRequestCallbackData*)data;
        auto model_resource = cbData->live2dRequest->model_resource = SkrNew<skr_live2d_model_resource_t>();
        auto model_setting = model_resource->model_setting 
            = SkrNew<L2DF::CubismModelSettingJson>(cbData->settingBlob->get_data(), (L2DF::csmSizeInt)cbData->settingBlob->get_size());
        cbData->settingBlob.reset();
        // setup models & expressions count
        if (auto _ = skr::String((const char8_t*)model_setting->GetModelFileName()); !_.is_empty())
        {
            cbData->model_count = 1;
        }
        if (auto _ = skr::String((const char8_t*)model_setting->GetPhysicsFileName()); !_.is_empty())
        {
            cbData->phys_count = 1;
        }
        if (auto _ = skr::String((const char8_t*)model_setting->GetPoseFileName()); !_.is_empty())
        {
            cbData->pose_count = 1;
        }
        if (auto _ = skr::String((const char8_t*)model_setting->GetUserDataFile()); !_.is_empty())
        {
            cbData->usr_data_count = 1;
        }
        cbData->expression_count = model_setting->GetExpressionCount();
        for (uint32_t i = 0; i < (uint32_t)model_setting->GetMotionGroupCount(); i++)
        {
            auto group = model_setting->GetMotionGroupName(i);
            cbData->motion_count += model_setting->GetMotionCount(group);
        }
        // load model & physics & pose & eyeblinks & breath & usrdata
        auto model = model_resource->model = SkrNew<L2DF::csmUserModel>();
        model->request(cbData->ioService, cbData);
        // load expressions
        auto expressions = model_resource->expression_map = SkrNew<L2DF::csmExpressionMap>();
        expressions->request(cbData->ioService, cbData);
        // load motions
        auto motions = model_resource->motion_map = SkrNew<L2DF::csmMotionMap>();
        motions->request(cbData->ioService, cbData);
    }, callbackData);
    callbackData->live2dRequest = live2dRequest;
    callbackData->ioService = ioService;
    // TODO: replace this with newer VFS API
    skr::String l2dHomePathStr;
    {
        SkrZoneScopedN("ioRAM Live2D Path Calc");
        auto l2dPath = skr::filesystem::path(path);
        l2dHomePathStr = l2dPath.parent_path().u8string().c_str();
        callbackData->u8HomePath = l2dHomePathStr.c_str();
    }
    rq->use_async_complete();
    callbackData->settingBlob = ioService->request(rq, &live2dRequest->settingsRequest);
}
#endif

void skr_live2d_model_update(skr_live2d_model_resource_id live2d_resource, float delta_time)
{
    if (live2d_resource)
    {
        live2d_resource->model->update(live2d_resource->motion_map, delta_time);
    }
}

const uint32_t* skr_live2d_model_get_sorted_drawable_list(skr_live2d_model_resource_id live2d_resource)
{
    if (live2d_resource)
    {
        return live2d_resource->model->get_sorted_drawlist();
    }
    return nullptr;
}

void skr_live2d_model_get_drawable_colors(skr_live2d_model_resource_id model, uint32_t drawable_index, 
    struct skr_float4_t* multiply_color, struct skr_float4_t* screen_color)
{
    auto _model = model->model->GetModel();
    const auto _multiply_color = _model->GetMultiplyColor(drawable_index);
    const auto _screen_color = _model->GetScreenColor(drawable_index);
    const float opacity = _model->GetDrawableOpacity(drawable_index);
    if (multiply_color) 
    {
        multiply_color->x = _multiply_color.R;
        multiply_color->y = _multiply_color.G;
        multiply_color->z = _multiply_color.B;
        multiply_color->w = _multiply_color.A;
        multiply_color->w *= opacity;
    }
    if (screen_color)
    {
        screen_color->x = _screen_color.R;
        screen_color->y = _screen_color.G;
        screen_color->z = _screen_color.B;
        screen_color->w = _screen_color.A;
    } 
}

const skr_live2d_vertex_pos_t* skr_live2d_model_get_drawable_vertex_positions(skr_live2d_model_resource_id live2d_resource, uint32_t drawable_index, uint32_t* out_count)
{
    auto model = live2d_resource->model->GetModel();
    if (!model)
    {
        SKR_LOG_ERROR(u8"no valid model");
        return nullptr;
    }
    auto positions = model->GetDrawableVertexPositions(drawable_index);
    if (out_count) *out_count = model->GetDrawableVertexCount(drawable_index);
    static_assert(sizeof(*positions) == sizeof(skr_live2d_vertex_pos_t));
    return (skr_live2d_vertex_pos_t*)positions;
}

const skr_live2d_vertex_uv_t* skr_live2d_model_get_drawable_vertex_uvs(skr_live2d_model_resource_id live2d_resource, uint32_t drawable_index, uint32_t* out_count)
{
    auto model = live2d_resource->model->GetModel();
    if (!model)
    {
        SKR_LOG_ERROR(u8"no valid model");
        return nullptr;
    }
    auto uvs = model->GetDrawableVertexUvs(drawable_index);
    if (out_count) *out_count = model->GetDrawableVertexCount(drawable_index);
    static_assert(sizeof(*uvs) == sizeof(skr_live2d_vertex_uv_t));
    return (skr_live2d_vertex_uv_t*)uvs;
}

bool skr_live2d_model_get_drawable_is_visible(skr_live2d_model_resource_id live2d_resource, uint32_t drawable_index)
{
    auto model = live2d_resource->model->GetModel();
    if (!model)
    {
        SKR_LOG_ERROR(u8"no valid model");
        return false;
    }
    return model->GetDrawableDynamicFlagIsVisible(drawable_index);
}

void skr_live2d_model_free(skr_live2d_model_resource_id live2d_resource)
{
    SkrDelete(live2d_resource->model_setting);
    SkrDelete(live2d_resource->model);
    SkrDelete(live2d_resource->expression_map);
    SkrDelete(live2d_resource->motion_map);
    SkrDelete(live2d_resource);
}