#include "MPShared/components.h"
#include "SkrScene/scene.h"
#include "containers/vector.hpp"
#include "MPShared/world_delta.h"
#include "ecs/type_builder.hpp"
#include "MPShared/shared.h"
#include "utils/log.h"
#include "ecs/array.hpp"

#include "world_delta_helper.hpp"

template<class T>
void RegisterSimpleComponent()
{
    constexpr auto builder = +[](dual_chunk_view_t view, const T& comp, skr_binary_writer_t& archive)
    {
        skr::binary::Archive(&archive, comp);
    };
    RegisterComponentDeltaBuilder(dual_id_of<T>::get(), &BuildDelta<T, builder>);
    constexpr auto applier = +[](dual_chunk_view_t view, T& comp, skr_binary_reader_t& archive)
    {
        skr::binary::Archive(&archive, comp);
    };
    RegisterComponentDeltaApplier(dual_id_of<T>::get(), &ApplyDelta<T, applier>);
}

dual_type_set_t GetNetworkComponents()
{
    static dual::static_type_set_T<skr_translation_comp_t, skr_scale_comp_t, skr_rotation_comp_t, CController, CMovement, CSphereCollider2D, CBall, CHealth, CSkill> set;
    return set.get();
}

dual_type_index_t GetNetworkComponent(uint8_t index)
{
    auto type = GetNetworkComponents();
    SKR_ASSERT(index < type.length);
    return type.data[index];
}

uint8_t GetNetworkComponentIndex(dual_type_index_t type)
{
    auto types = GetNetworkComponents();
    auto iter = std::find(types.data, types.data + types.length, type);
    SKR_ASSERT(iter != types.data + types.length);
    return static_cast<uint8_t>(iter - types.data);
}

//TODO: crash with clang-cl + release, don't know why 
#pragma optimize("", off)
void InitializeNetworkComponents()
{
    {
        static skr::binary::VectorSerdeConfig<float> translationSerdeConfig = { 10000 };
        //auto translationHistory =  RegisterHistoryComponent<skr_translation_comp_t>();
        constexpr auto builder = +[](dual_chunk_view_t view, const skr_translation_comp_t& comp, skr_translation_comp_t_History& historyComp, bool initialMap, skr_binary_writer_t& archive)
        {
            uint32_t full = initialMap || historyComp.deltaAccumulated > 40.f;
            archive.write_bits(&full, 1);
            if(full)
            {
                skr::binary::Archive(&archive, comp.value, translationSerdeConfig);
                historyComp.position.x = comp.value.x;
                historyComp.position.y = comp.value.z;
                historyComp.deltaAccumulated = 0.f;
            }
            else
            {
                skr_float2_t delta;
                delta.x = comp.value.x - historyComp.position.x;
                delta.y = comp.value.z - historyComp.position.y;
                skr::binary::Archive(&archive, delta, translationSerdeConfig);
                historyComp.position.x = comp.value.x;
                historyComp.position.y = comp.value.z;
                historyComp.deltaAccumulated += std::abs(delta.x) + std::abs(delta.y);
            }
            return false;
        };
        RegisterComponentDeltaBuilder(dual_id_of<skr_translation_comp_t>::get(), &BuildDelta<skr_translation_comp_t, builder, skr_translation_comp_t_History, true>, dual_id_of<skr_translation_comp_t_History>::get());
        constexpr auto applier = +[](dual_chunk_view_t view, skr_translation_comp_t& comp, skr_binary_reader_t& archive)
        {
            uint32_t full = 0;
            archive.read_bits(&full, 1);
            if(full)
            {
                skr::binary::Archive(&archive, comp.value, translationSerdeConfig);
            }
            else
            {
                skr_float2_t delta;
                skr::binary::Archive(&archive, delta, translationSerdeConfig);
                comp.value.x += delta.x;
                comp.value.z += delta.y;
            }
        };
        RegisterComponentDeltaApplier(dual_id_of<skr_translation_comp_t>::get(), &ApplyDelta<skr_translation_comp_t, applier, true>);
    }
    RegisterSimpleComponent<skr_scale_comp_t>();
    {
        //optimize1: only send needed value (yaw)
        //optimize2: compress value
        constexpr auto builder = +[](dual_chunk_view_t view, const skr_rotation_comp_t& comp, skr_binary_writer_t& archive)
        {
            float rot = comp.euler.yaw;
            uint16_t rot16 = static_cast<uint16_t>(rot * 65536.0f / 360.0f);
            skr::binary::Archive(&archive, rot16);
        };
        RegisterComponentDeltaBuilder(dual_id_of<skr_rotation_comp_t>::get(), &BuildDelta<skr_rotation_comp_t, builder>);
        constexpr auto applier = +[](dual_chunk_view_t view, skr_rotation_comp_t& comp, skr_binary_reader_t& archive)
        {
            uint16_t rot16;
            skr::binary::Archive(&archive, rot16);
            comp.euler.yaw = rot16 * 360.0f / 65536.0f;
        };
        RegisterComponentDeltaApplier(dual_id_of<skr_rotation_comp_t>::get(), &ApplyDelta<skr_rotation_comp_t, applier>);
    }
    RegisterSimpleComponent<CSphereCollider2D>();
    RegisterSimpleComponent<CHealth>();
    RegisterSimpleComponent<CSkill>();
    {
        constexpr auto builder = +[](dual_chunk_view_t view, const CController& comp, skr_binary_writer_t& archive)
        {
            skr::binary::Archive(&archive, comp);
        };
        RegisterComponentDeltaBuilder(dual_id_of<CController>::get(), &BuildDelta<CController, builder>);
        constexpr auto applier = +[](dual_chunk_view_t view, CController& comp, skr_binary_reader_t& archive)
        {
            skr::binary::Archive(&archive, comp);
            comp.playerId = comp.localPlayerId;
        };
        RegisterComponentDeltaApplier(dual_id_of<CController>::get(), &ApplyDelta<CController, applier>);
    }
    {
        //optimize1: serialize bitpacked value
        static auto velocitySerdeConfig = skr::binary::VectorSerdeConfig<float>{100};
        constexpr auto builder = +[](dual_chunk_view_t view, const CMovement& comp, skr_binary_writer_t& archive)
        {
            skr::binary::Archive(&archive, comp.velocity, velocitySerdeConfig);
        };
        RegisterComponentDeltaBuilder(dual_id_of<CMovement>::get(), &BuildDelta<CMovement, builder, void, true>);
        constexpr auto applier = +[](dual_chunk_view_t view, CMovement& comp, skr_binary_reader_t& archive)
        {
            skr::binary::Archive(&archive, comp.velocity, velocitySerdeConfig);
        };
        RegisterComponentDeltaApplier(dual_id_of<CMovement>::get(), &ApplyDelta<CMovement, applier, true>);
    }
}
#pragma optimize("", on)