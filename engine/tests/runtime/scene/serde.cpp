#include "SkrCore/log.h"
#include "SkrContainers/vector.hpp"
#include "SkrSceneCore/transform_system.h"
#include <SkrCore/serialize/json_archive.hpp>

#include "SkrTestFramework/framework.hpp"

struct SceneSerdeTests
{
protected:
    SceneSerdeTests()
    {
        skr_log_set_level(SKR_LOG_LEVEL_INFO);
    }
};

template <typename T>
struct TestSceneType
{
    TestSceneType(T value)
        : value(value)
    {
        skr::String json_str;

        // write
        {
            auto writer = skr::ArWriteJson::Create();

            {
                skr::Archive::ObjectScope obj_scope{ writer };
                SKR_FAST_CHECK(obj_scope.is_success(), );
                SKR_FAST_CHECK(writer.key_value(u8"key", value), );
            }

            // as string
            SKR_FAST_CHECK(writer.root()->write_to_string(json_str), );
        }

        SKR_LOG_INFO(u8"SCENE TYPE JSON: %s", json_str.c_str_raw());

        // read
        {
            auto reader = skr::ArReadJson::ReadBuffer(json_str.data(), json_str.size());

            T read_value;
            {
                skr::Archive::ObjectScope obj_scope{ reader };
                SKR_FAST_CHECK(obj_scope.is_success(), );
                SKR_FAST_CHECK(reader.key_value(u8"key", read_value), );
            }

            EXPECT_EQ(value, read_value);
        }
    }
    T value;
};

TEST_CASE_METHOD(SceneSerdeTests, "json")
{
    TestSceneType<skr::scene::ScaleComponent>(skr::scene::ScaleComponent(1.0, 2.0, 3.0));
    TestSceneType<skr::scene::ScaleComponent>(skr::scene::ScaleComponent(5.f, 4.f, 3.f));
    TestSceneType<skr::scene::PositionComponent>(skr::scene::PositionComponent(15.f, 42.f, 34.f));
    TestSceneType<skr::scene::PositionComponent>(skr::scene::PositionComponent(5.f, 4.f, 3.f));
    TestSceneType<skr::scene::RotationComponent>(skr::scene::RotationComponent(5.f, 4.f, 3.f));
}
