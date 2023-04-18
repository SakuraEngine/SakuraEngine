#include "MPShared/client_world.h"
#include "utils/make_zeroed.hpp"
#include "EASTL/fixed_vector.h"
#include "utils/parallel_for.hpp"
#include "utils/log.h"

#include "ecs/type_builder.hpp"
#include "ecs/set.hpp"
#include "json/writer.h"
#include "SkrScene/scene.h"
#include "MPShared/components.h"
#include "steam/isteamnetworkingutils.h"
#include "steam/isteamnetworkingsockets.h"
#include "steam/steamnetworkingsockets.h"
#include "math/vector.h"
#include "lz4.h"
MPClientWorld::MPClientWorld()
{

}

MPClientWorld::~MPClientWorld()
{
}

void MPClientWorld::Initialize()
{
    MPGameWorld::Initialize();
    dualJ_bind_storage(storage);
    snapshotStorage = nullptr;
    skr_init_hires_timer(&timer);
    input.inputs.resize(1);
    lastTime = predictedGameTime = skr_hires_timer_get_seconds(&timer, false);
    snapshotQuery = dualQ_from_literal(storage, "[in]CNetwork");
    healthQuery = dualQ_from_literal(storage, "[in]CHealth, [has]CController");
    InitializeNetworkComponents();
    worldDeltaApplier = CreateWorldDeltaApplier();
    worldDeltaApplier->Initialize(storage, 
    [this](dual_storage_t*, dual_entity_t entity, skr_guid_t prefab, dual_entity_type_t* type)
    {
        return SpawnPrefab(prefab, entity, *type);
    },
    [this](dual_storage_t*, dual_entity_t entity)
    {
        DestroyEntity(entity);
    });
    authoritative = false;
}

void MPClientWorld::Shutdown()
{
    dualQ_release(snapshotQuery);
    if (snapshotStorage)
    {
        dualS_release(snapshotStorage);
    }
    dualJ_unbind_storage(storage);
    MPGameWorld::Shutdown();
}

void MPClientWorld::ReceiveWorldDelta(const void* data, size_t dataLength)
{
    ZoneScopedN("MPClientWorld::ReceiveWorldDelta");
    skr::span<uint8_t> span;
    skr::vector<uint8_t> decompressedData;
    {
        ZoneScopedN("Decompress");
        bandwidthCounter.AddRecord(dataLength);
        uint64_t size = *(uint64_t*)data;
        data = (uint8_t*)data + sizeof(uint64_t);
        dataLength -= sizeof(uint64_t);
        decompressedData.resize(LZ4_COMPRESSBOUND(size));
        auto decompressedSize = LZ4_decompress_safe((const char*)data, (char*)decompressedData.data(), dataLength, decompressedData.size());
        SKR_ASSERT(decompressedSize == size);
        span = {(uint8_t*)decompressedData.data(), size};
    }
    skr::binary::SpanReader reader{span, 0};
    skr_binary_reader_t archive(reader);
    skr::binary::Read(&archive, worldDelta);
    
    auto forward = worldDelta.frame - verifiedFrame;
    if(currentFrame > worldDelta.frame)
    {
        std::move(&predictedInputs[forward], &predictedInputs[predictedFrame - verifiedFrame], predictedInputs);
    }
    verifiedFrame = worldDelta.frame;
    if(firstFrame)
    {
        firstFrame = false;
        predictedFrame = currentFrame = verifiedFrame;
    }

    // {
    //     skr_json_writer_t jsonWriter(4);
    //     skr::json::Write(&jsonWriter, worldDelta.blob);
    //     SKR_LOG_DEBUG("Receiving delta %s : ", jsonWriter.Str().c_str());
    // }
}

dual_entity_t MPClientWorld::SpawnPrefab(skr_resource_handle_t prefab, dual_entity_t entity, dual_entity_type_t extension)
{
    dual_entity_t result;
    dual::type_builder_t builder;
    builder.with<CPrefab, CNetwork, CGhost>();
    builder.with(extension.type.data, extension.type.length);
    dual_entity_type_t type = make_zeroed<dual_entity_type_t>();
    type.type = builder.build();
    auto initialize = [&](dual_chunk_view_t* view) {
        auto prefabs = (CPrefab*)dualV_get_component_ro(view, dual_id_of<CPrefab>::get());
        auto networks = (CNetwork*)dualV_get_component_ro(view, dual_id_of<CNetwork>::get());
        auto ghosts = (CGhost*)dualV_get_component_ro(view, dual_id_of<CGhost>::get());
        

        prefabs[0].prefab = prefab;
        ghosts[0].mappedEntity = DUAL_NULL_ENTITY;
        networks[0].serverEntity = entity;
        result = dualV_get_entities(view)[0];
    };
    dualS_allocate_type(storage, &type, 1, DUAL_LAMBDA(initialize));
    return result;
}

void MPClientWorld::DestroyEntity(dual_entity_t entity)
{
    auto freeFunc = [&](dual_chunk_view_t* view) {
        dualS_destroy(storage, view);
    };
    dual_chunk_view_t view;
    dualS_access(storage, entity, &view);
    freeFunc(&view);
}

void MPClientWorld::ApplyWorldDelta()
{
    ZoneScopedN("MPClientWorld::ApplyWorldDelta");
    entity_map_t map;
    if(predictionEnabled)
    {
        // 0. roll back authority data
        RollBack();
    }
    // 1. apply delta to authority data
    worldDeltaApplier->ApplyDelta(worldDelta.blob, map);
    if(predictionEnabled)
    {
        // 2. snapshot new authority data
        Snapshot();
    }
    currentFrame = verifiedFrame;
}

void MPClientWorld::SendInput()
{
    // for(auto& i : input.inputs)
    // {
    //     i.fire = true;
    // }
    input.frame = inputFrame;
    skr::vector<uint8_t> buffer;
    buffer.resize(sizeof(uint32_t));
    *(uint32_t*)&buffer[0] = (uint32_t)MPEventType::Input;
    skr::binary::VectorWriter writer{&buffer};
    skr_binary_writer_t archive(writer);
    skr::binary::Write(&archive, input);
    SteamNetworkingSockets()->SendMessageToConnection(serverConnection, buffer.data(), buffer.size(), k_nSteamNetworkingSend_Reliable, nullptr);
}

skr::vector<dual_chunk_view_t> merge_views(skr::vector<dual_chunk_view_t>& views)
{
    skr::vector<dual_chunk_view_t> merged;
    std::sort(views.begin(), views.end(), [](const dual_chunk_view_t& lhs, const dual_chunk_view_t& rhs)
    {
        return lhs.start < rhs.start;
    });
    for(auto view : views)
    {
        if(merged.empty())
        {
            merged.push_back(view);
        }
        else 
        {
            auto& curr = merged.back();
            if(view.start < curr.start + curr.count)
            {
                curr.count = std::max(curr.count, view.start+view.count-curr.start);
            }
            else 
            {
                merged.push_back(view);
            }
        }
    }
    return merged;
}

void SnapshotComponent(void* dst, const void* src, dual_type_index_t type, uint32_t count)
{
    //TODO: handle reference
    auto desc = dualT_get_desc(type);
    memcpy(dst, src, desc->size * count);
}

void ApplyComponent(void* dst, const void* src, dual_type_index_t type, uint32_t count)
{
    //TODO: handle reference
    auto desc = dualT_get_desc(type);
    memcpy(dst, src, desc->size * count);
}

void MPClientWorld::Snapshot()
{
    ZoneScopedN("MPClientWorld::Snapshot");
    if(snapshotStorage)
        dualS_reset(snapshotStorage);
    else
        snapshotStorage = dualS_create();
    //find out datas needed to snapshot by scan all gameplay related query
    //then create a snapshot entity to store those data
    auto callback = [&](dual_group_t* group) 
    {
        skr::flat_hash_map<dual_chunk_t*, skr::vector<dual_chunk_view_t>> views;
        dual::type_builder_t predictedBuilder;
        for(auto query : {movementQuery, controlQuery})
        {
            bool match = false;
            auto callback = [&](dual_chunk_view_t* view)
            {
                match = true;
                //match end point
                views[view->chunk].push_back(*view);
            };
            dualQ_get_views_group(query, group, DUAL_LAMBDA(callback));
            
            if(match)
            {
                dual_filter_t filter;
                dual_parameters_t params;
                dualQ_get(query, &filter, &params);
                for(int i=0; i<params.length; ++i)
                {
                    if(!params.accesses[i].readonly)
                        predictedBuilder.with(params.types[i]);
                }
            }
        }
        auto predictedType = predictedBuilder.build();
        
        dual_entity_type_t entType;
        dualG_get_type(group, &entType);
        std::vector<dual_type_index_t> buffer;
        buffer.resize(std::max(predictedType.length, entType.type.length));
        auto predictedInView = dual::set_utils<dual_type_index_t>::intersect(entType.type, predictedType, buffer.data());
        dual::type_builder_t builder;
        builder.with(predictedInView.data, predictedInView.length);
        builder.with<CSnapshot>();
        auto snapshotType = make_zeroed<dual_entity_type_t>();
        snapshotType.type = builder.build();

        for(auto& pair : views)
        {
            skr::vector<dual_chunk_view_t> merged = merge_views(pair.second);
            
            for(auto view : merged)
            {
                auto callback = [&](dual_chunk_view_t* sview)
                {
                    for(int i=0; i<predictedInView.length; ++i)
                    {
                        auto type = predictedInView.data[i];
                        void* dst = (void*)dualV_get_owned_ro(sview, type);
                        const void* src = dualV_get_owned_ro(&view, type);
                        SnapshotComponent(dst, src, type, sview->count);
                    }
                    auto owners = dualV_get_owned_ro(sview, dual_id_of<CSnapshot>::get());
                    const dual_entity_t* entities = dualV_get_entities(&view);
                    std::memcpy((void*)owners, entities, sizeof(dual_entity_t) * sview->count);
                    view.start += sview->count;
                    view.count -= sview->count;
                };
                dualS_allocate_type(snapshotStorage, &snapshotType, view.count, DUAL_LAMBDA(callback));
            }
        }
    };
    dualQ_get_groups(snapshotQuery, DUAL_LAMBDA(callback));
}

void MPClientWorld::RollBack()
{
    ZoneScopedN("MPClientWorld::RollBack");
    if(snapshotStorage)
    {
        dual_type_index_t snapshotT = dual_id_of<CSnapshot>::get();
        dual_filter_t filter = make_zeroed<dual_filter_t>();
        filter.all.data = &snapshotT;
        filter.all.length = 1;
        dual_meta_filter_t meta = make_zeroed<dual_meta_filter_t>();
        auto callback = [&](dual_chunk_view_t* sview) {
            auto owners = (const dual_entity_t*)dualV_get_owned_ro(sview, snapshotT);
            dual_entity_type_t setype;
            dualG_get_type(dualC_get_group(sview->chunk), &setype);
            int i = 0;
            auto callback = [&](dual_chunk_view_t* dview)
            {
                for(int k=0; k<setype.type.length; ++k)
                {
                    auto type = setype.type.data[k];
                    if(type == snapshotT)
                        continue;
                    auto dst = dualV_get_owned_rw(dview, type);
                    dual_chunk_view_t current = {dview->chunk, dview->start + i, dview->count - i};
                    auto src = dualV_get_owned_ro(&current, type);
                    ApplyComponent(dst, src, type, dview->count);
                }
                i += dview->count;
            };
            dualS_batch(storage, owners, sview->count, DUAL_LAMBDA(callback));
        };
        dualS_query(snapshotStorage, &filter, &meta, DUAL_LAMBDA(callback));
    }
}

void MPClientWorld::UpdateTimeScale(double deltaTime)
{
    deltaTime = std::min(GetTickInterval(), deltaTime);
    SteamNetConnectionRealTimeStatus_t status;
    SteamNetworkingSockets()->GetConnectionRealTimeStatus(serverConnection, &status, 0, nullptr);
    float ping = status.m_nPing / 1000.f;
    uint64_t shouldPredictFrames = ping / GetTickInterval() + 1;
    uint64_t currentPredictedFrame = predictedFrame - verifiedFrame;
    if(predictedFrame < verifiedFrame)
    {
        //speed up
        timeScale -= 0.5 * deltaTime;
        timeScale = std::max(0.5, timeScale);
        return;
    }
    if (currentPredictedFrame > shouldPredictFrames)
    {
        //speed down
        timeScale += 0.5 * deltaTime;
        timeScale = std::min(1.5, timeScale);
    }
    else if (currentPredictedFrame < shouldPredictFrames)
    {
        //speed up
        timeScale -= 0.5 * deltaTime;
        timeScale = std::max(0.5, timeScale);
    }
    else 
    {
        //towards 1
        if(timeScale > 1)
        {
            timeScale -= 0.5 * deltaTime;
            timeScale = std::max(1.0, timeScale);
        }
        else if(timeScale < 1)
        {
            timeScale += 0.5 * deltaTime;
            timeScale = std::min(1.0, timeScale);
        }
    }
}

double MPClientWorld::GetBytePerSecond()
{
    return bandwidthCounter.GetBytePerSecond();
}

void MPClientWorld::RollForward()
{
    ZoneScopedN("MPClientWorld::RollForward");
    if(predictionEnabled)
    {
        while (currentFrame < predictedFrame)
        {
            Tick(predictedInputs[currentFrame - verifiedFrame]);
            currentFrame++;
        }
    }
}

void MPClientWorld::SkipFrame()
{
    predictedGameTime += GetTickInterval() * timeScale;
}


void MPClientWorld::SetPredictionEnabled(bool enabled)
{
    predictionEnabled = enabled;
    if(predictionEnabled)
    {
        Snapshot();
        predictedFrame = currentFrame;
        predictedGameTime = skr_hires_timer_get_seconds(&timer, false);
    }
    else 
    {
        RollBack();
        currentFrame = verifiedFrame;
    }
}

bool MPClientWorld::Update()
{
    ZoneScopedN("MPClientWorld::Update");
    bool updated = false;
    auto currentTime = skr_hires_timer_get_seconds(&timer, false);
    double deltaTime = currentTime - lastTime;
    lastTime = currentTime;
    currentGameTime = currentTime;
    if(predictionEnabled)
    {
        while (currentGameTime - predictedGameTime > GetTickInterval() * timeScale)
        {
            if(predictedFrame < currentFrame)
            {
                predictedFrame++;
                predictedGameTime += GetTickInterval() * timeScale;
            }
            else 
            {
                uint64_t predictedFrames = (currentFrame > verifiedFrame ? currentFrame - verifiedFrame : 0);
                if(predictedFrames >= MaxPredictFrame)
                    break;
                predictedGameTime += GetTickInterval() * timeScale;
                predictedInputs[currentFrame - verifiedFrame] = input;
                currentFrame++;
                Tick(input);
                updated = true;
            }
        }
        predictedFrame = currentFrame;
        UpdateTimeScale(deltaTime);
    }

    if(inputFrame != currentFrame)
    {
        inputFrame = currentFrame;
        SendInput();
    }
    return updated;
}

float MPClientWorld::GetPlayerHealth()
{
    float result = 0;
    auto DisplayHealth = [&](dual_chunk_view_t* view)
    {
        auto healths = (const CHealth*)dualV_get_owned_ro(view, dual_id_of<CHealth>::get());
        auto entities = dualV_get_entities(view);
        for(int i=0; i<view->count; ++i)
        {
            result = healths[i].health;
        }
    };
    dualQ_get_views(healthQuery, DUAL_LAMBDA(DisplayHealth));
    return result;
}