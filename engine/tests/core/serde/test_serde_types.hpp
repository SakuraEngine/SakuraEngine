#pragma once
// TODO. 移动到专门的 test 文件
#include "SkrBase/config.h"
#include "SkrRTTR/iobject.hpp"
#include <SkrContainers/map.hpp>
#include <SkrContainers/set.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrContainers/variant.hpp>
#include <SkrContainers/array.hpp>
#include <SkrContainers/string.hpp>
#include "test_serde_types.generated.h"
#include <SkrBase/math.h>

namespace test_serde
{
template <typename K, typename V>
using TestComplexUsingMap1 = skr::Map<K, V>;
template <typename K, typename V>
using TestComplexUsingMap2 = TestComplexUsingMap1<K, skr::Vector<V>>;
template <typename V>
using TestComplexUsingMap3 = TestComplexUsingMap2<skr::String, skr::vec_t<V, 4>>;

struct [[sattr(
    guid = "a3847f6a-05b0-4049-a619-e687ef4bc856"
    serde = @enable
)]] TestJson
{
    SKR_GENERATE_BODY(TestJson)

    // in json: {"serde_normal": <number>}
    int32_t serde_normal;

    // in json: {"class": <number>}
    [[sattr(serde.alias = "class")]]
    int32_t klass;

    // will not appare in json
    [[sattr(serde = @disable)]]
    int32_t disabled;

    // must exist in json
    [[sattr(serde.required = true)]]
    int32_t required_value;

private:
    int32_t _private_member;
};

struct [[sattr(
    guid = "eb2a94ea-e0f4-4b93-9d3f-17d13ea3af14"
    serde = @enable
)]] TestBin
{
    SKR_GENERATE_BODY(TestBin)

    // in json: {"serde_normal": <number>}
    int32_t serde_normal;

    // will not appare in bin
    [[sattr(serde = @disable)]]
    int32_t disabled;

private:
    int32_t _private_member;
};

enum class [[sattr(
    guid = "d4f3c8b2-2b6e-4f3c-9c7b-1e2f3d4c5b6a"
    serde = @enable
)]] EComplexEnum : int32_t
{
    ValueA,
    ValueB,
    ValueC,
    ValueD,
    ValueE,
    ValueF,
    ValueG,
    ValueH,
    ValueI,
    ValueJ,
    ValueK,
    ValueL,
    ValueM,
    ValueN,
    ValueO,
    ValueP,
    ValueQ,
    ValueR,
    ValueS,
    ValueT,
    ValueU,
    ValueV,
    ValueW,
    ValueX,
    ValueY,
};

struct [[sattr(
    guid = "fefbadd0-1a15-4480-b93f-eeb3543acf19"
    serde = @enable
)]] TestComplexTypeMember
{
    SKR_GENERATE_BODY(TestComplexTypeMember)

    // some primitive types
    bool bool_1;
    int8_t i8_1;
    uint8_t u8_1;
    int16_t i16_1;
    uint16_t u16_1;
    int32_t i32_1;
    uint32_t u32_1;
    int64_t i64_1;
    uint64_t u64_1;
    float f32_1;
    double f64_1;

    // some math types
    skr::float3x3 mat_1;
    skr::float4x4 mat_2;
    skr::float4 vec_1;
    skr::float3 vec_2;
    skr::TransformF trans_1;
    skr::TransformD trans_2;
    skr::QuatD quat_1;

    // some variant types
    skr::variant<skr::float4, skr::String, int32_t> variant_1;

    // some container types
    skr::Vector<int32_t> vector_1;
    skr::Map<skr::String, skr::double4x4> map_1;
    skr::Set<skr::String> set_1;

    // GUIDs
    skr::Vector<skr::GUID> guids;

    // some enums
    EComplexEnum enum_1;
    EComplexEnum enum_2;
    EComplexEnum enum_3;
    EComplexEnum enum_4;
    EComplexEnum enum_5;
    EComplexEnum enum_6;
    EComplexEnum enum_7;
    EComplexEnum enum_8;

    // some native array
    skr::float3 native_array_1[3];
    skr::String native_array_2[20];

    // some skr::array
    skr::Array<int32_t, 4> skr_array_1;
    skr::Array<skr::String, 4> skr_array_2;

    // test complex using
    TestComplexUsingMap3<float> using_map_1;

    inline void fill_some_data()
    {
        // fill primitive types
        bool_1 = rand() % 2;
        i8_1 = rand() % 128;
        u8_1 = rand() % 256;
        i16_1 = rand() % 32768;
        u16_1 = rand() % 65536;
        i32_1 = rand();
        u32_1 = rand();
        i64_1 = ((int64_t)rand() << 32) | rand();
        u64_1 = ((uint64_t)rand() << 32) | rand();
        f32_1 = (float)rand() / RAND_MAX;
        f64_1 = (double)rand() / RAND_MAX;

        // fill math types
        mat_1 = { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f,
                  rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f,
                  rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f };
        mat_2 = skr::float4x4::eye(rand() % 10 / 10.0f);
        vec_1 = { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f };
        vec_2 = { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f };
        trans_1 = {
            { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f },
            { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f },
            { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f }
        };
        trans_2 = {
            { rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 100 / 10.0 },
            { rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 100 / 10.0 },
            { rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 100 / 10.0 }
        };
        quat_1 = { rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 100 / 10.0, rand() % 100 / 10.0 };

        // fill variant types
        auto idx = rand() % 3;
        switch (idx)
        {
        case 0:
            variant_1 = skr::float4{ rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f };
            break;
        case 1:
            variant_1 = skr::String(skr::format(u8"random_string_{}", rand() % 1000));
            break;
        case 2:
            variant_1 = rand();
            break;
        }

        // fill container types
        for (int i = 0; i < 100; i++)
        {
            vector_1.push_back(rand() % 100);
        }
        for (int i = 0; i < 20; i++)
        {
            auto key = skr::format(u8"key_{}", i);
            auto value = skr::float4x4::eye(rand() % 10 / 10.0f);
            map_1.add(std::move(key), std::move(value));
        }
        for (int i = 0; i < 20; i++)
        {
            auto value = skr::format(u8"set_value_{}", i);
            set_1.add(std::move(value));
        }

        // fill guids
        for (int i = 0; i < 1000; i++)
        {
            guids.push_back(skr::GUID::Create());
        }

        // fill enum types
        enum_1 = EComplexEnum::ValueA;
        enum_2 = EComplexEnum::ValueF;
        enum_3 = EComplexEnum::ValueK;
        enum_4 = EComplexEnum::ValueP;
        enum_5 = EComplexEnum::ValueB;
        enum_6 = EComplexEnum::ValueG;
        enum_7 = EComplexEnum::ValueL;
        enum_8 = EComplexEnum::ValueQ;

        // fill native array
        for (int i = 0; i < 3; i++)
        {
            native_array_1[i] = { rand() % 100 / 10.0f, rand() % 100 / 10.0f, rand() % 100 / 10.0f };
        }
        for (int i = 0; i < 20; i++)
        {
            native_array_2[i] = skr::format(u8"native_array_string_{}", i);
        }

        // fill skr::array
        for (int i = 0; i < 4; i++)
        {
            skr_array_1[i] = rand() % 100;
            skr_array_2[i] = skr::format(u8"skr_array_string_{}", i);
        }

        // fill complex using map
        for (int i = 0; i < 10; i++)
        {
            auto key = skr::format(u8"using_map_key_{}", i);
            skr::Vector<skr::float4> vec;
            for (int j = 0; j < 5; j++)
            {
                vec.push_back((float)(rand() % 100) / 10.0f);
            }
            using_map_1.add(std::move(key), std::move(vec));
        }
    }

    inline bool operator==(const TestComplexTypeMember& rhs) const
    {
        // compare primitive types
        if (bool_1 != rhs.bool_1) return false;
        if (i8_1 != rhs.i8_1) return false;
        if (u8_1 != rhs.u8_1) return false;
        if (i16_1 != rhs.i16_1) return false;
        if (u16_1 != rhs.u16_1) return false;
        if (i32_1 != rhs.i32_1) return false;
        if (u32_1 != rhs.u32_1) return false;
        if (i64_1 != rhs.i64_1) return false;
        if (u64_1 != rhs.u64_1) return false;
        if (f32_1 != rhs.f32_1) return false;
        if (f64_1 != rhs.f64_1) return false;

        // compare math types
        if (mat_1 != rhs.mat_1) return false;
        if (mat_2 != rhs.mat_2) return false;
        if (any(vec_1 != rhs.vec_1)) return false;
        if (any(vec_2 != rhs.vec_2)) return false;
        if (trans_1 != rhs.trans_1) return false;
        if (trans_2 != rhs.trans_2) return false;
        if (any(quat_1 != rhs.quat_1)) return false;

        // compare variant types
        if (variant_1.index() != rhs.variant_1.index()) return false;
        switch (variant_1.index())
        {
        case 0:
            if (any(skr::get<0>(variant_1) != skr::get<0>(rhs.variant_1))) return false;
            break;
        case 1:
            if (skr::get<1>(variant_1) != skr::get<1>(rhs.variant_1)) return false;
            break;
        case 2:
            if (skr::get<2>(variant_1) != skr::get<2>(rhs.variant_1)) return false;
            break;
        }

        // compare container types
        if (vector_1 != rhs.vector_1) return false;
        if (map_1.size() != rhs.map_1.size()) return false;
        for (auto& [k, v] : map_1)
        {
            auto found = rhs.map_1.find(k);
            if (!found) return false;
            if (v != found.value()) return false;
        }
        if (set_1.size() != rhs.set_1.size()) return false;

        // compare enum types
        if (enum_1 != rhs.enum_1) return false;
        if (enum_2 != rhs.enum_2) return false;
        if (enum_3 != rhs.enum_3) return false;
        if (enum_4 != rhs.enum_4) return false;
        if (enum_5 != rhs.enum_5) return false;
        if (enum_6 != rhs.enum_6) return false;
        if (enum_7 != rhs.enum_7) return false;
        if (enum_8 != rhs.enum_8) return false;

        // compare native array
        for (int i = 0; i < 3; i++)
        {
            if (any(native_array_1[i] != rhs.native_array_1[i])) return false;
        }
        for (int i = 0; i < 20; i++)
        {
            if (native_array_2[i] != rhs.native_array_2[i]) return false;
        }

        // compare skr::array
        for (int i = 0; i < 4; i++)
        {
            if (skr_array_1[i] != rhs.skr_array_1[i]) return false;
            if (skr_array_2[i] != rhs.skr_array_2[i]) return false;
        }

        // compare complex using map
        if (using_map_1.size() != rhs.using_map_1.size()) return false;
        for (auto& [k, v] : using_map_1)
        {
            auto found = rhs.using_map_1.find(k);
            if (!found) return false;
            if (v.size() != found.value().size()) return false;
            for (size_t i = 0; i < v.size(); i++)
            {
                if (any(v[i] != found.value()[i])) return false;
            }
        }

        return true;
    }
};

struct [[sattr(
    guid = "81a8fd61-4ed6-4e69-ac1e-575cee195045"
    serde = @enable
)]] TestComplexType
{
    SKR_GENERATE_BODY(TestComplexType)

    skr::Vector<TestComplexTypeMember> members;
    skr::Map<skr::String, TestComplexTypeMember> member_map;

    inline void fill_some_data(uint32_t amount = 1000)
    {
        members.clear();
        member_map.clear();
        for (uint32_t i = 0; i < amount; i++)
        {
            TestComplexTypeMember member;
            member.fill_some_data();
            members.push_back(std::move(member));

            TestComplexTypeMember member2;
            member2.fill_some_data();
            member_map.add(skr::format(u8"member_{}", i), std::move(member2));
        }
    }

    inline bool operator==(const TestComplexType& rhs) const
    {
        if (members != rhs.members) return false;
        if (member_map.size() != rhs.member_map.size()) return false;
        for (auto& [k, v] : member_map)
        {
            auto found = rhs.member_map.find(k);
            if (!found) return false;
            if (v != found.value()) return false;
        }
        return true;
    }
};
} // namespace test_serde