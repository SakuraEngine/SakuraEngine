#include "SkrCore/log.h"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrContainers/vector.hpp"

#include "SkrTestFramework/framework.hpp"

struct JSONSerdeTests {
protected:
    JSONSerdeTests()
    {
        skr_log_set_level(SKR_LOG_LEVEL_INFO);
    }
};

void _EXPECT_OK(skr::json::ReadResult&& r)
{
    using namespace skr::json;
    r.and_then([]() {
        EXPECT_EQ(true, true);
    })
    .error_then([](ErrorCode error) {
        EXPECT_EQ(true, false);
    });
}
#define EXPECT_OK _EXPECT_OK
#define DEFAULT_EQ nullptr

template <typename T, typename EQ>
struct TestTypeSerde {
    TestTypeSerde(const char8_t* key, const T& v, EQ eq)
        : value(v)
    {
        skr::json::Writer writer(2);
        {
            EXPECT_OK(writer.StartObject());
            writer.Key(key);
            skr::json::Write(&writer, value);
            EXPECT_OK(writer.EndObject());
        }

        auto json = writer.Write();
        SKR_LOG_INFO(u8"TYPE SERDE JSON: %s", json.c_str());
        
        skr::json::Reader reader(json.view());
        reader.StartObject();
        {
            reader.Key(key);

            T _value;
            skr::json::Read(&reader, _value);
            if constexpr (std::is_same_v<EQ, nullptr_t>)
                EXPECT_EQ(value, _value);
            else
                eq(value, _value);
        }
        reader.EndObject();
    }

    T value;
};

TEST_CASE_METHOD(JSONSerdeTests, "types")
{
    // primitives
    {
        TestTypeSerde(u8"i32", 123, DEFAULT_EQ);
        TestTypeSerde(u8"u32", 123u, DEFAULT_EQ);
        TestTypeSerde(u8"i64", 123ll, DEFAULT_EQ);
        TestTypeSerde(u8"u64", 123ull, DEFAULT_EQ);
        TestTypeSerde(u8"f32", 123.f, DEFAULT_EQ);
        TestTypeSerde(u8"f64", 123.0, DEFAULT_EQ);
        TestTypeSerde(u8"bool", true, DEFAULT_EQ);
        TestTypeSerde(u8"string", skr::String(u8"hello"), DEFAULT_EQ);
    }
    // guid
    {
        skr_guid_t guid;
        skr_make_guid(&guid);
        TestTypeSerde(u8"guid", guid, DEFAULT_EQ);
    }
    // md5
    {
        skr_md5_t md5;
        skr_make_md5(u8"123", 3, &md5);
        TestTypeSerde(u8"md5", md5, DEFAULT_EQ);
    }
    // float2/3/4
    {
        skr_float2_t v2 = { 1.f, 2.f };
        TestTypeSerde(u8"float2", v2, DEFAULT_EQ);
    }
    {
        skr_float3_t v3 = { 1.f, 2.f, 3.f };
        TestTypeSerde(u8"float3", v3, DEFAULT_EQ);
    }
    {
        skr_float4_t v4 = { 1.f, 2.f, 3.f, 4.f };
        TestTypeSerde(u8"float4", v4, DEFAULT_EQ);
    }
}

TEST_CASE_METHOD(JSONSerdeTests, "containers")
{
    // vector
    {
        skr::Vector<uint64_t> values  = { 0x12345678, 0x87654321 };
        TestTypeSerde(u8"vec<u64>", values, 
            [](const auto& v1, const auto& v2) 
            {
                EXPECT_EQ(v1[0], v2[0]);
                EXPECT_EQ(v1[1], v2[1]);
            }
        );
    }
    // vec-vec
    {
        skr::Vector<skr::Vector<uint64_t>> values = {
            { 0x12345678, 0x87654321 },
            { 0x87654321, 0x12345678 }
        };
        TestTypeSerde(u8"vec<vec<u64>>", values, 
            [](const auto& v1, const auto& v2) 
            {
                EXPECT_EQ(v1[0][0], v2[0][0]);
                EXPECT_EQ(v1[0][1], v2[0][1]);
                EXPECT_EQ(v1[1][0], v2[1][0]);
                EXPECT_EQ(v1[1][1], v2[1][1]);
            }
        );
    }
    // string-hashmap
    {
        skr::FlatHashMap<skr::String, int32_t, skr::Hash<skr::String>> stringMap;
        stringMap.emplace(u8"key1", 123);
        stringMap.emplace(u8"key2", 234);
        stringMap.emplace(u8"key3", 456);

        TestTypeSerde(u8"hashmap<string, i32>", stringMap, 
            [](auto& v1, auto& v2) 
            {
                EXPECT_EQ(v1[u8"key1"], v2[u8"key1"]);
                EXPECT_EQ(v1[u8"key2"], v2[u8"key2"]);
                EXPECT_EQ(v1[u8"key3"], v2[u8"key3"]);
            }
        );
    }
    // id-hashmap
    {
        skr::FlatHashMap<skr_guid_t, int32_t, skr::Hash<skr_guid_t>> idMap;
        skr::Vector<skr_guid_t> guids;
        guids.resize_zeroed(3);
        for (uint32_t i = 0; i < 3; i++)
        {
            skr_make_guid(&guids[i]);
            idMap.emplace(guids[i], i);
        }

        TestTypeSerde(u8"hashmap<guid, i32>", idMap, 
            [&](auto& v1, auto& v2) 
            {
                EXPECT_EQ(v1[guids[0]], v2[guids[0]]);
                EXPECT_EQ(v1[guids[1]], v2[guids[1]]);
                EXPECT_EQ(v1[guids[2]], v2[guids[2]]);
            }
        );
    }
}