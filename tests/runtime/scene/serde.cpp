#include "SkrCore/log.h"
#include "SkrSerde/json/writer.h"
#include "SkrSerde/json/reader.h"
#include "SkrContainers/vector.hpp"
#include "SkrScene/scene.h"

#include "SkrTestFramework/framework.hpp"

struct SceneSerdeTests {
protected:
    SceneSerdeTests()
    {
        skr_log_set_level(SKR_LOG_LEVEL_INFO);
    }
};

template <typename T>
struct TestSceneType {
    TestSceneType(T value)
        : value(value)
    {
        SJsonWriter writer(1);
        {
            writer.StartObject();
            writer.Key(u8"key");
            skr::json::Write(&writer, value);
            writer.EndObject();
        }
        {
            auto json  = writer.Write();
            SKR_LOG_INFO(u8"SCENE TYPE JSON: %s", json.c_str());

            SJsonReader reader(json.view());
            reader.StartObject();
            {
                reader.Key(u8"key");

                T _value;
                skr::json::Read(&reader, _value);
                EXPECT_EQ(value, _value);
            }
            reader.EndObject();
        }
    }
    T value;
};

TEST_CASE_METHOD(SceneSerdeTests, "json")
{
    TestSceneType<skr_scale_comp_t>(skr_scale_comp_t { skr_float3_t(1.f, 2.f, 3.f) });
    TestSceneType<skr_scale_comp_t>(skr_scale_comp_t { skr_float3_t(5.f, 4.f, 3.f) });
    TestSceneType<skr_translation_comp_t>(skr_translation_comp_t { skr_float3_t(15.f, 42.f, 34.f) });
    TestSceneType<skr_translation_comp_t>(skr_translation_comp_t { skr_float3_t(5.f, 4.f, 3.f) });
    TestSceneType<skr_rotation_comp_t>(skr_rotation_comp_t { skr_rotator_t(5.f, 4.f, 3.f) });
}
