#include "MPShared/server_world.h"
#include "MPShared/components.h"
#include "utils/make_zeroed.hpp"
#include "EASTL/fixed_vector.h"
#include "utils/parallel_for.hpp"
#include "utils/log.h"

#include "ecs/type_builder.hpp"
#include "ecs/set.hpp"
#include "ecs/array.hpp"
#include "json/writer.h"
#include "simdjson.h"
#include "steam/isteamnetworkingsockets.h"
#include "steam/steamnetworkingsockets.h"
#include "lz4.h"

MPServerWorld::MPServerWorld()
{
}

MPServerWorld::~MPServerWorld()
{
}

int MPServerWorld::GetPlayerConnection(int player, int* localId)
{
    for(int i=0; i<playerMap.size(); ++i)
    {
        for(int j=0; j<playerMap[i].size(); ++j)
        {
            auto id = playerMap[i][j];
            
            if(id == player)
            {
                if(localId)
                    *localId = j;
                return i;
            }
        }
    }
    return -1;
}

void MPServerWorld::AddConnection(HSteamNetConnection connection)
{
    connections.push_back(connection);
    worldDelta.emplace_back();
    playerMap.emplace_back().emplace_back(playerId);
    SpawnPlayerEntity(playerId, connections.size() - 1, 0);
    playerId++;
}

void MPServerWorld::SpawnPlayerEntity(int player, int connectionId, int localPlayerId)
{
    // allocate 1 movable cubes
    auto playerT_builder = make_zeroed<dual::type_builder_t>();
    playerT_builder
        .with<skr_translation_comp_t, skr_rotation_comp_t, skr_scale_comp_t, 
        CMovement, CSphereCollider2D, CWeapon, CHealth, CSkill,
        CController, CPrefab, CAuth, CAuthTypeData, CRelevance>().with(DUAL_COMPONENT_DIRTY);
    float spawnArea = 100.f;
    // allocate renderable
    auto playerT = make_zeroed<dual_entity_type_t>();
    playerT.type = playerT_builder.build();
    auto primSetup = [&](dual_chunk_view_t* view) {
        auto translations = (skr_translation_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_translation_comp_t>::get());
        auto rotations = (skr_rotation_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_rotation_comp_t>::get());
        auto scales = (skr_scale_comp_t*)dualV_get_owned_ro(view, dual_id_of<skr_scale_comp_t>::get());
        auto movements = (CMovement*)dualV_get_owned_ro(view, dual_id_of<CMovement>::get());
        auto controllers = (CController*)dualV_get_owned_ro(view, dual_id_of<CController>::get());
        auto collidors = (CSphereCollider2D*)dualV_get_owned_ro(view, dual_id_of<CSphereCollider2D>::get());
        auto weapons = (CWeapon*)dualV_get_owned_ro(view, dual_id_of<CWeapon>::get());
        auto healths = (CHealth*)dualV_get_owned_ro(view, dual_id_of<CHealth>::get());
        auto skills = (CSkill*)dualV_get_owned_ro(view, dual_id_of<CSkill>::get());
        auto relevances = (CRelevance*)dualV_get_owned_ro(view, dual_id_of<CRelevance>::get());
        for (uint32_t i = 0; i < view->count; i++)
        {
            //randomize spawn position
            translations[i].value = {rand() % (int)spawnArea - spawnArea / 2.f, 0.f, rand() % (int)spawnArea - spawnArea / 2.f };
            rotations[i].euler = { 0.f, 0.f, 0.f };
            scales[i].value = { 10.f, 1.f, 10.f };
            movements[i].speed = movements[i].baseSpeed = 30.f;
            controllers[i].playerId = player;
            controllers[i].serverPlayerId = player;
            controllers[i].connectionId = connectionId;
            controllers[i].localPlayerId = localPlayerId;
            collidors[i].radius = 8.f;
            weapons[i].fireRate = 0.1f;
            weapons[i].fireTimer = 0.f;
            healths[i].health = 100.f;
            healths[i].maxHealth = 100.f;
            skills[i].cooldown = 5.f;
            skills[i].duration = 2.f;
            skills[i].speedMultiplier = 2.f;
            relevances[i].mask.reset();
            relevances[i].mask.flip();
        }
    };
    dualS_allocate_type(storage, &playerT, 1, DUAL_LAMBDA(primSetup));
}

void MPServerWorld::InitializeScene()
{
    
}

void MPServerWorld::Initialize()
{
    MPGameWorld::Initialize();
    InitializeNetworkComponents();
    worldDeltaBuilder = CreateWorldDeltaBuilder();
    worldDeltaBuilder->Initialize(storage);
    deadQuery = dualQ_from_literal(storage, "[inout]CAuth, [has]dead");
    skr_init_hires_timer(&timer);
    lastGameTime = skr_hires_timer_get_seconds(&timer, false);
    gameFrame = 0;
    dualJ_bind_storage(storage);
    authoritative = true;
}

void MPServerWorld::Shutdown()
{
    dualQ_release(deadQuery);
    dualJ_unbind_storage(storage);
    SkrDelete(worldDeltaBuilder);
    MPGameWorld::Shutdown();
}

void MPServerWorld::GenerateWorldDelta()
{
    worldDeltaBuilder->GenerateDelta(worldDelta);
}


skr::vector<uint8_t> MPServerWorld::SerializeWorldDelta(const MPWorldDeltaViewBuilder& deltaBuilder)
{
    MPWorldDelta delta;
    delta.arena = skr::binary::make_arena<MPWorldDeltaView>(delta.blob, deltaBuilder);
    delta.frame = gameFrame;
    delta.time = lastGameTime;
    skr::vector<uint8_t> buffer;
    skr::binary::VectorWriter writer{&buffer};
    skr_binary_writer_t archive(writer);
    skr::binary::Write(&archive, delta);
    return buffer;
}

void MPServerWorld::SendWorldDelta()
{
    int count = connections.size();
    for(int i=0; i<count; ++i)
    {
        auto result = SerializeWorldDelta(worldDelta[i]);
        skr::vector<uint8_t> compressed;
        compressed.resize(LZ4_COMPRESSBOUND(result.size()));
        int compressedSize = LZ4_compress_default((const char*)result.data(), (char*)compressed.data(), result.size(), compressed.size());
        SKR_ASSERT(compressedSize > 0);
        skr::vector<uint8_t> data;
        data.resize(sizeof(uint32_t) + sizeof(uint64_t) + compressedSize);
        auto current = &data[0];
        *(uint32_t*)current = (uint32_t)MPEventType::SyncWorld;
        current += sizeof(uint32_t);
        *(uint64_t*)current = result.size();
        current += sizeof(uint64_t);
        memcpy(current, compressed.data(), compressedSize);

        {
            skr::vector<uint8_t> decompressedData;
            decompressedData.resize(result.size());
            int decompressedSize = LZ4_decompress_safe((const char*)current, (char*)decompressedData.data(), compressedSize, decompressedData.size());
            SKR_ASSERT(decompressedSize == decompressedData.size());
        }
        SteamNetworkingSockets()->SendMessageToConnection(connections[i], data.data(), data.size(), k_nSteamNetworkingSend_Reliable, nullptr);
    }
}

MPInputFrame ReceiveInput(const void* data, size_t dataLength)
{
    MPInputFrame result;
    skr::span<uint8_t> span{(uint8_t*)data, dataLength};
    skr::binary::SpanReader reader{span, 0};
    skr_binary_reader_t archive(reader);
    skr::binary::Read(&archive, result);
    return result;
}

void MPServerWorld::AccumulateInput(uint32_t connectionId, const MPInputFrame &input)
{
    uint64_t frame = input.frame;
    auto maxQueuedFrameCount = sizeof(queuedInputs) / sizeof(queuedInputs[0]);
    if((frame > gameFrame) && (frame - gameFrame > maxQueuedFrameCount))
    {
        SKR_LOG_WARN("input from %d is too far in the future! currentFrame: %d, recievedFrame %d", connectionId, gameFrame, frame);
        return;
    }
    uint64_t effectiveFrame = frame > gameFrame ? frame : gameFrame;
    auto& target = queuedInputs[effectiveFrame - gameFrame];
    target.inputs.resize(playerId);
    int i = 0;
    for(auto& playerInput : input.inputs)
    {
        auto serverPlayer = playerMap[connectionId][i++];
        target.inputs[serverPlayer] = playerInput;
    }
}

SHiresTimer get_timer()
{
    SHiresTimer result;
    skr_init_hires_timer(&result);
    return result;
}

void MPServerWorld::Update()
{
    if(connections.empty())
        return;
    //accumulate input
    for(auto& connection : connections)
    {
        SteamNetworkingMessage_t *pMessages[10];
        int r = 0;
        do
        {
            r = SteamNetworkingSockets()->ReceiveMessagesOnConnection( connection, pMessages, 10 );
            SKR_ASSERT(r != -1);
            for(int i=0; i<r; ++i)
            {
                auto pMessage = pMessages[i];
                auto data = pMessage->GetData();
                auto type = *(MPEventType*)data;
                switch(type)
                {
                    case MPEventType::Input:
                    {
                        auto input = ReceiveInput((char*)data + sizeof(MPEventType), pMessage->GetSize() - sizeof(MPEventType));
                        int j = 0;
                        for(; j<connections.size() && connections[j] != pMessage->GetConnection(); ++j);
                        AccumulateInput(j, input);
                    }
                    break;
                    default:
                        SKR_UNREACHABLE_CODE();
                        break;
                }
            }
        }
        while (r);
    }

    double currentGameTime = skr_hires_timer_get_seconds(&timer, false);
    
    while(currentGameTime - lastGameTime > serverTickInterval)
    {
        lastGameTime += serverTickInterval;
        queuedInputs[0].inputs.resize(playerId);
        Tick(queuedInputs[0]);
        GenerateWorldDelta();
        dual::array_comp_T<dual_group_t*, 16> deadGroups;
        auto getDeadGroups = [&](dual_group_t* group)
        {
            deadGroups.emplace_back(group);
        };
        dualQ_get_groups(deadQuery, DUAL_LAMBDA(getDeadGroups));
        dual_delta_type_t delta = make_zeroed<dual_delta_type_t>();
        dual_type_index_t types[] = { dual_id_of<CAuth>::get() };
        delta.removed.type = { types, 1 };
        for(auto& group : deadGroups)
            dualS_cast_group_delta(storage, group, &delta, nullptr, nullptr);
        dualJ_wait_all();
        gameFrame++;
        SendWorldDelta();
        std::move(queuedInputs+1, queuedInputs + 128, queuedInputs); // shift
    }
    LogNetworkStatics();
}

void MPServerWorld::LogNetworkStatics()
{
    static auto statusDisplayTimer = get_timer();
    if(skr_hires_timer_get_seconds(&statusDisplayTimer, false) > 2)
    {
        skr_hires_timer_reset(&statusDisplayTimer);
        float bandwidth = 0;
        float estimatedBandwidth = 0;
        for(auto& connection : connections)
        {
            SteamNetConnectionRealTimeStatus_t status;
            SteamNetworkingSockets()->GetConnectionRealTimeStatus(connection, &status, 0, nullptr);
            bandwidth += status.m_flOutBytesPerSec;
            estimatedBandwidth = std::max((float)status.m_nSendRateBytesPerSecond, estimatedBandwidth);
        }
        SKR_LOG_INFO("bandwidth: %f, estimatedBandwidth: %f", bandwidth, estimatedBandwidth);
    }
}