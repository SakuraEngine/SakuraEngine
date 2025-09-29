// Math Operations on 3D Coordinate Transformations
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/math.hpp"

TEST_CASE("DefaultCoordinate")
{
    using namespace skr;
    {
        // Test default coordinate system
        // x right
        // z forward
        // y up
        auto f = skr::float3::forward();
        CHECK(f[0] == 0.0f);
        CHECK(f[1] == 0.0f);
        CHECK(f[2] == 1.0f);
        auto r = skr::float3::right();
        CHECK(r[0] == 1.0f);
        CHECK(r[1] == 0.0f);
        CHECK(r[2] == 0.0f);
        auto u = skr::float3::up();
        CHECK(u[0] == 0.0f);
        CHECK(u[1] == 1.0f);
        CHECK(u[2] == 0.0f);
    }
}

template <typename QuatType, typename VecType>
void check_relative_transform()
{
    QuatType quat_child = QuatType::Euler(
        skr::radians(10.0f),
        skr::radians(20.0f),
        skr::radians(30.0f)
    );
    QuatType quat_parent = QuatType::Euler(
        skr::radians(20.0f),
        skr::radians(30.0f),
        skr::radians(40.0f)
    );
    QuatType quat_world    = quat_child * quat_parent;
    QuatType quat_relative = relative(quat_parent, quat_world);
    REQUIRE(all(nearly_equal(quat_relative, quat_child)));
    VecType v{ 1, 1, 1 };
    VecType v_child    = v * quat_child;
    VecType v_relative = v * quat_relative;
    REQUIRE(all(nearly_equal(v_relative, v_child)));
}

TEST_CASE("RelativeTransform")
{
    using namespace skr;
    SUBCASE("QuatF")
    {
        check_relative_transform<QuatF, float3>();
    }
    SUBCASE("QuatD")
    {
        check_relative_transform<QuatD, double3>();
    }
    SUBCASE("no neg scale")
    {
        // for TransformF
        {
            QuatF quat_a = QuatF::Euler(
                radians(10.0f),
                radians(20.0f),
                radians(30.0f)
            );
            QuatF quat_b = QuatF::Euler(
                radians(20.0f),
                radians(30.0f),
                radians(40.0f)
            );

            TransformF transform_child{
                quat_a,
                { 1.0f, 2.0f, 3.0f },
                { 3.0f, 2.0f, 1.0f }
            };
            TransformF transform_parent{
                quat_b,
                { 4.0f, 5.0f, 6.0f },
                { 6.0f, 5.0f, 4.0f }
            };

            TransformF transform_world    = transform_child * transform_parent;
            TransformF transform_relative = relative(transform_parent, transform_world);

            REQUIRE(nearly_equal(transform_relative, transform_child));
        }

        // for TransformD
        {
            QuatD quat_a = QuatD::Euler(
                radians(10.0),
                radians(20.0),
                radians(30.0)
            );
            QuatD quat_b = QuatD::Euler(
                radians(20.0),
                radians(30.0),
                radians(40.0)
            );

            TransformD transform_child{
                quat_a,
                { 1.0, 2.0, 3.0 },
                { 3.0, 2.0, 1.0 }
            };
            TransformD transform_parent{
                quat_b,
                { 4.0, 5.0, 6.0 },
                { 6.0, 5.0, 4.0 }
            };

            TransformD transform_world    = transform_child * transform_parent;
            TransformD transform_relative = relative(transform_parent, transform_world);

            REQUIRE(nearly_equal(transform_relative, transform_child));
        }
    }

    SUBCASE("with neg scale")
    {
        // for TransformF
        {
            QuatF quat_a = QuatF::Euler(
                radians(10.0f),
                radians(20.0f),
                radians(30.0f)
            );
            QuatF quat_b = QuatF::Euler(
                radians(20.0f),
                radians(30.0f),
                radians(40.0f)
            );

            TransformF transform_child{
                quat_a,
                { 1.0f, 2.0f, 3.0f },
                { -3.0f, 2.0f, 1.0f }
            };
            TransformF transform_parent{
                quat_b,
                { 4.0f, 5.0f, 6.0f },
                { 6.0f, 5.0f, -4.0f }
            };
            TransformF transform_world    = transform_child * transform_parent;
            TransformF transform_relative = relative(transform_parent, transform_world);

            // REQUIRE(nearly_equal(transform_relative.rotation, transform_child.rotation));
            REQUIRE(all(nearly_equal(transform_relative.position, transform_child.position)));
            REQUIRE(all(nearly_equal(transform_relative.scale, transform_child.scale)));

            // float3 v{ 1.f, 1.f, 1.f };
            // float3 v_child    = mul_point(v, transform_child);
            // float3 v_relative = mul_point(v, transform_relative);
            // REQUIRE(all(nearly_equal(v_relative, v_child, 1.e-2f)));
        }

        // for TransformD
        {
            QuatD quat_a = QuatD::Euler(
                radians(10.0),
                radians(20.0),
                radians(30.0)
            );
            QuatD quat_b = QuatD::Euler(
                radians(20.0),
                radians(30.0),
                radians(40.0)
            );

            TransformD transform_child{
                quat_a,
                { 1.0, 2.0, 3.0 },
                { -3.0, 2.0, 1.0 }
            };
            TransformD transform_parent{
                quat_b,
                { 4.0, 5.0, 6.0 },
                { 6.0, 5.0, -4.0 }
            };
            TransformD transform_world    = transform_child * transform_parent;
            TransformD transform_relative = relative(transform_parent, transform_world);

            // REQUIRE(nearly_equal(transform_relative.rotation, transform_child.rotation));
            REQUIRE(all(nearly_equal(transform_relative.position, transform_child.position)));
            REQUIRE(all(nearly_equal(transform_relative.scale, transform_child.scale)));

            // double3 v{ 1.f, 1.f, 1.f };
            // double3 v_child    = mul_point(v, transform_child);
            // double3 v_relative = mul_point(v, transform_relative);
            // REQUIRE(all(nearly_equal(v_relative, v_child)));
        }
    }
}

TEST_CASE("RotatorQuatConvert")
{
    using namespace skr;

    // for float
    {
        RotatorF rotator{
            radians(10.0f),
            radians(20.0f),
            radians(30.0f)
        };
        QuatF    quat{ rotator };
        RotatorF conv_back{ quat };
        QuatF    conv_back_quat{ conv_back };
        REQUIRE(all(nearly_equal(rotator, conv_back)));
        REQUIRE(all(nearly_equal(quat, conv_back_quat)));
    }

    // for double
    {
        RotatorD rotator{
            radians(10.0),
            radians(20.0),
            radians(30.0)
        };
        QuatD    quat{ rotator };
        RotatorD conv_back{ quat };
        QuatD    conv_back_quat{ conv_back };
        REQUIRE(all(nearly_equal(rotator, conv_back)));
        REQUIRE(all(nearly_equal(quat, conv_back_quat)));
    }
}