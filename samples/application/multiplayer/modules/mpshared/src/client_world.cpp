#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrRT/ecs/set.hpp"
#include "SkrSerde/binary/reader.h"
#include "SkrSerde/binary/writer.h"
#include "SkrScene/scene.h"

#include "MPShared/components.h"
#include "MPShared/client_world.h"

#include "steam/isteamnetworkingsockets.h"
#include "lz4.h"

#include "SkrProfile/profile.h"

MPClientWorld::MPClientWorld()
{
}

MPClientWorld::~MPClientWorld()
{
}

void MPClientWorld::Initialize()
{
    MPGameWorld::Initialize();
    sugoiJ_bind_storage(storage);
    snapshotStorage = nullptr;
    skr_init_hires_timer(&timer);
    input.inputs.resize_default(1);
    lastTime = predictedGameTime = skr_hires_timer_get_seconds(&timer, false);
    snapshotQuery                = sugoiQ_from_literal(storage, u8"[in]CNetwork");
    healthQuery                  = sugoiQ_from_literal(storage, u8"[in]CHealth, [has]CController");
    InitializeNetworkComponents();
    worldDeltaApplier = CreateWorldDeltaApplier();
    worldDeltaApplier->Initialize(storage, [this](sugoi_storage_t*, sugoi_entity_t entity, skr_guid_t prefab, sugoi_entity_type_t* type) { return SpawnPrefab(prefab, entity, *type); }, [this](sugoi_storage_t*, sugoi_entity_t entity) { DestroyEntity(entity); });
    authoritative = false;
}

void MPClientWorld::Shutdown()
{
    sugoiQ_release(snapshotQuery);
    if (snapshotStorage)
    {
        sugoiS_release(snapshotStorage);
    }
    sugoiJ_unbind_storage(storage);
    MPGameWorld::Shutdown();
}

void MPClientWorld::ReceiveWorldDelta(const void* data, size_t dataLength)
{
    SkrZoneScopedN("MPClientWorld::ReceiveWorldDelta");
    skr::span<uint8_t>   span;
    skr::Vector<uint8_t> decompressedData;
    {
        SkrZoneScopedN("Decompress");
        bandwidthCounter.AddRecord(dataLength);
        uint64_t size = *(uint64_t*)data;
        bandwidthBeforeCompressCounter.AddRecord(size);
        data = (uint8_t*)data + sizeof(uint64_t);
        dataLength -= sizeof(uint64_t);
        compressRatio = (double)dataLength / size;
        decompressedData.resize_default(LZ4_COMPRESSBOUND(size));
        auto decompressedSize = LZ4_decompress_safe((const char*)data, (char*)decompressedData.data(), dataLength, decompressedData.size());
        SKR_ASSERT(decompressedSize == size);
        span = { (uint8_t*)decompressedData.data(), size };
    }
    skr::binary::SpanReaderBitpacked reader{ span, 0 };
    skr_binary_reader_t              archive(reader);
    skr::binary::Read(&archive, worldDelta);

    auto forward = worldDelta.frame - verifiedFrame;
    if (currentFrame > worldDelta.frame)
    {
        std::move(&predictedInputs[forward], &predictedInputs[predictedFrame - verifiedFrame], predictedInputs);
    }
    verifiedFrame = worldDelta.frame;
    if (firstFrame)
    {
        firstFrame     = false;
        predictedFrame = currentFrame = verifiedFrame;
    }

    // {
    //     skr_json_writer_t jsonWriter(4);
    //     skr::json::Write(&jsonWriter, worldDelta.blob);
    //     SKR_LOG_DEBUG(u8"Receiving delta %s : ", jsonWriter.Str().c_str());
    // }
}

sugoi_entity_t MPClientWorld::SpawnPrefab(skr_resource_handle_t prefab, sugoi_entity_t entity, sugoi_entity_type_t extension)
{
    sugoi_entity_t        result;
    sugoi::type_builder_t builder;
    builder.with<CPrefab, CNetwork, CGhost>();
    builder.with(extension.type.data, extension.type.length);
    sugoi_entity_type_t type = make_zeroed<sugoi_entity_type_t>();
    type.type                = builder.build();
    auto initialize          = [&](sugoi_chunk_view_t* view) {
        auto prefabs  = (CPrefab*)sugoiV_get_component_ro(view, sugoi_id_of<CPrefab>::get());
        auto networks = (CNetwork*)sugoiV_get_component_ro(view, sugoi_id_of<CNetwork>::get());
        auto ghosts   = (CGhost*)sugoiV_get_component_ro(view, sugoi_id_of<CGhost>::get());

        prefabs[0].prefab        = prefab;
        ghosts[0].mappedEntity   = SUGOI_NULL_ENTITY;
        networks[0].serverEntity = entity;
        result                   = sugoiV_get_entities(view)[0];
    };
    sugoiS_allocate_type(storage, &type, 1, SUGOI_LAMBDA(initialize));
    return result;
}

void MPClientWorld::DestroyEntity(sugoi_entity_t entity)
{
    auto freeFunc = [&](sugoi_chunk_view_t* view) {
        sugoiS_destroy(storage, view);
    };
    sugoi_chunk_view_t view;
    sugoiS_access(storage, entity, &view);
    freeFunc(&view);
}

void MPClientWorld::ApplyWorldDelta()
{
    SkrZoneScopedN("MPClientWorld::ApplyWorldDelta");
    entity_map_t map;
    if (predictionEnabled)
    {
        // 0. roll back authority data
        RollBack();
    }
    // 1. apply delta to authority data
    worldDeltaApplier->ApplyDelta(worldDelta.blob, map);
    if (predictionEnabled)
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
    skr::Vector<uint8_t> buffer;
    buffer.resize_default(sizeof(uint32_t));
    *(uint32_t*)&buffer[0] = (uint32_t)MPEventType::Input;
    skr::binary::VectorWriter writer{ &buffer };
    skr_binary_writer_t       archive(writer);
    skr::binary::Write(&archive, input);
    SteamNetworkingSockets()->SendMessageToConnection(serverConnection, buffer.data(), buffer.size(), k_nSteamNetworkingSend_Reliable, nullptr);
}

skr::Vector<sugoi_chunk_view_t> merge_views(skr::Vector<sugoi_chunk_view_t>& views)
{
    skr::Vector<sugoi_chunk_view_t> merged;
    std::sort(views.begin(), views.end(), [](const sugoi_chunk_view_t& lhs, const sugoi_chunk_view_t& rhs) {
        return lhs.start < rhs.start;
    });
    for (auto view : views)
    {
        if (merged.empty())
        {
            merged.add(view);
        }
        else
        {
            auto& curr = merged[merged.size() - 1];
            if (view.start < curr.start + curr.count)
            {
                curr.count = std::max(curr.count, view.start + view.count - curr.start);
            }
            else
            {
                merged.add(view);
            }
        }
    }
    return merged;
}

void SnapshotComponent(void* dst, const void* src, sugoi_type_index_t type, uint32_t count)
{
    // TODO: handle reference
    auto desc = sugoiT_get_desc(type);
    memcpy(dst, src, desc->size * count);
}

void ApplyComponent(void* dst, const void* src, sugoi_type_index_t type, uint32_t count)
{
    // TODO: handle reference
    auto desc = sugoiT_get_desc(type);
    memcpy(dst, src, desc->size * count);
}

void MPClientWorld::Snapshot()
{
    SkrZoneScopedN("MPClientWorld::Snapshot");
    if (snapshotStorage)
        sugoiS_reset(snapshotStorage);
    else
        snapshotStorage = sugoiS_create();
    // find out datas needed to snapshot by scan all gameplay related query
    // then create a snapshot entity to store those data
    auto callback = [&](sugoi_group_t* group) {
        skr::FlatHashMap<sugoi_chunk_t*, skr::Vector<sugoi_chunk_view_t>> views;
        sugoi::type_builder_t                                             predictedBuilder;
        for (auto query : { movementQuery.query, controlQuery.query })
        {
            bool match    = false;
            auto callback = [&](sugoi_chunk_view_t* view) {
                match = true;
                // match end point
                views[view->chunk].add(*view);
            };
            sugoiQ_get_views_group(query, group, SUGOI_LAMBDA(callback));

            if (match)
            {
                sugoi_filter_t     filter;
                sugoi_parameters_t params;
                sugoiQ_get(query, &filter, &params);
                for (int i = 0; i < params.length; ++i)
                {
                    if (!params.accesses[i].readonly)
                        predictedBuilder.with(params.types[i]);
                }
            }
        }
        auto predictedType = predictedBuilder.build();

        sugoi_entity_type_t entType;
        sugoiG_get_type(group, &entType);
        std::vector<sugoi_type_index_t> buffer;
        buffer.resize(std::max(predictedType.length, entType.type.length));
        auto                  predictedInView = sugoi::set_utils<sugoi_type_index_t>::intersect(entType.type, predictedType, buffer.data());
        sugoi::type_builder_t builder;
        builder.with(predictedInView.data, predictedInView.length);
        builder.with<CSnapshot>();
        auto snapshotType = make_zeroed<sugoi_entity_type_t>();
        snapshotType.type = builder.build();

        for (auto& pair : views)
        {
            skr::Vector<sugoi_chunk_view_t> merged = merge_views(pair.second);

            for (auto view : merged)
            {
                auto callback = [&](sugoi_chunk_view_t* sview) {
                    for (int i = 0; i < predictedInView.length; ++i)
                    {
                        auto        type = predictedInView.data[i];
                        void*       dst  = (void*)sugoiV_get_owned_ro(sview, type);
                        const void* src  = sugoiV_get_owned_ro(&view, type);
                        SnapshotComponent(dst, src, type, sview->count);
                    }
                    auto                  owners   = sugoiV_get_owned_ro(sview, sugoi_id_of<CSnapshot>::get());
                    const sugoi_entity_t* entities = sugoiV_get_entities(&view);
                    std::memcpy((void*)owners, entities, sizeof(sugoi_entity_t) * sview->count);
                    view.start += sview->count;
                    view.count -= sview->count;
                };
                sugoiS_allocate_type(snapshotStorage, &snapshotType, view.count, SUGOI_LAMBDA(callback));
            }
        }
    };
    sugoiQ_get_groups(snapshotQuery, SUGOI_LAMBDA(callback));
}

void MPClientWorld::RollBack()
{
    SkrZoneScopedN("MPClientWorld::RollBack");
    if (snapshotStorage)
    {
        sugoi_type_index_t snapshotT = sugoi_id_of<CSnapshot>::get();
        sugoi_filter_t     filter    = make_zeroed<sugoi_filter_t>();
        filter.all.data              = &snapshotT;
        filter.all.length            = 1;
        sugoi_meta_filter_t meta     = make_zeroed<sugoi_meta_filter_t>();
        auto                callback = [&](sugoi_chunk_view_t* sview) {
            auto                owners = (const sugoi_entity_t*)sugoiV_get_owned_ro(sview, snapshotT);
            sugoi_entity_type_t setype;
            sugoiG_get_type(sugoiC_get_group(sview->chunk), &setype);
            int  i        = 0;
            auto callback = [&](sugoi_chunk_view_t* dview) {
                for (int k = 0; k < setype.type.length; ++k)
                {
                    auto type = setype.type.data[k];
                    if (type == snapshotT)
                        continue;
                    auto               dst     = sugoiV_get_owned_rw(dview, type);
                    sugoi_chunk_view_t current = { sview->chunk, sview->start + i, sview->count - i };
                    auto               src     = sugoiV_get_owned_ro(&current, type);
                    ApplyComponent(dst, src, type, dview->count);
                }
                i += dview->count;
            };
            sugoiS_batch(storage, owners, sview->count, SUGOI_LAMBDA(callback));
        };
        sugoiS_query(snapshotStorage, &filter, &meta, SUGOI_LAMBDA(callback));
    }
}

void MPClientWorld::UpdateTimeScale(double deltaTime)
{
    deltaTime = std::min(GetTickInterval(), deltaTime);
    SteamNetConnectionRealTimeStatus_t status;
    SteamNetworkingSockets()->GetConnectionRealTimeStatus(serverConnection, &status, 0, nullptr);
    float    ping                  = status.m_nPing / 1000.f;
    uint64_t shouldPredictFrames   = ping / GetTickInterval() + 1;
    uint64_t currentPredictedFrame = predictedFrame - verifiedFrame;
    if (predictedFrame < verifiedFrame)
    {
        // speed up
        timeScale -= 0.5 * deltaTime;
        timeScale = std::max(0.5, timeScale);
        return;
    }
    if (currentPredictedFrame > shouldPredictFrames)
    {
        // speed down
        timeScale += 0.5 * deltaTime;
        timeScale = std::min(1.5, timeScale);
    }
    else if (currentPredictedFrame < shouldPredictFrames)
    {
        // speed up
        timeScale -= 0.5 * deltaTime;
        timeScale = std::max(0.5, timeScale);
    }
    else
    {
        // towards 1
        if (timeScale > 1)
        {
            timeScale -= 0.5 * deltaTime;
            timeScale = std::max(1.0, timeScale);
        }
        else if (timeScale < 1)
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

double MPClientWorld::GetBytePerSecondBeforeCompress()
{
    return bandwidthBeforeCompressCounter.GetBytePerSecond();
}

double MPClientWorld::GetCompressRatio()
{
    return compressRatio;
}

void MPClientWorld::RollForward()
{
    SkrZoneScopedN("MPClientWorld::RollForward");
    if (predictionEnabled)
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
    if (predictionEnabled)
    {
        Snapshot();
        predictedFrame    = currentFrame;
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
    SkrZoneScopedN("MPClientWorld::Update");
    bool   updated     = false;
    auto   currentTime = skr_hires_timer_get_seconds(&timer, false);
    double deltaTime   = currentTime - lastTime;
    lastTime           = currentTime;
    currentGameTime    = currentTime;
    if (predictionEnabled)
    {
        while (currentGameTime - predictedGameTime > GetTickInterval() * timeScale)
        {
            if (predictedFrame < currentFrame)
            {
                predictedFrame++;
                predictedGameTime += GetTickInterval() * timeScale;
            }
            else
            {
                uint64_t predictedFrames = (currentFrame > verifiedFrame ? currentFrame - verifiedFrame : 0);
                if (predictedFrames >= MaxPredictFrame)
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

    if (inputFrame != currentFrame)
    {
        inputFrame = currentFrame;
        SendInput();
    }
    return updated;
}

float MPClientWorld::GetPlayerHealth()
{
    float result        = 0;
    auto  DisplayHealth = [&](sugoi_chunk_view_t* view) {
        auto healths = (const CHealth*)sugoiV_get_owned_ro(view, sugoi_id_of<CHealth>::get());
        // auto entities = sugoiV_get_entities(view);
        for (int i = 0; i < view->count; ++i)
        {
            result = healths[i].health;
        }
    };
    sugoiQ_get_views(healthQuery, SUGOI_LAMBDA(DisplayHealth));
    return result;
}