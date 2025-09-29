// test suite for linear algebra operations
#include "SkrTestFramework/framework.hpp"
#include "SkrBase/math.hpp"
#include <type_traits>

TEST_CASE("VectorOps")
{
    using namespace skr;
    SUBCASE("float2_vector_addition")
    {
        // float2 vector addition
        float2 v1{ 1.0f, 2.0f };
        float2 v2{ 3.0f, 4.0f };
        float2 v3 = v1 + v2;
        REQUIRE(v3.x == approx(4.0f));
        REQUIRE(v3.y == approx(6.0f));
    }
    SUBCASE("float3_vector_addition")
    {
        // float3 vector addition
        float3 v1{ 1.0f, 2.0f, 3.0f };
        float3 v2{ 4.0f, 5.0f, 6.0f };
        float3 v3 = v1 + v2;
        REQUIRE(v3.x == 5.0f);
        REQUIRE(v3.y == 7.0f);
        REQUIRE(v3.z == 9.0f);
    }
    SUBCASE("float4_vector_addition")
    {
        // float4 vector addition
        float4 v1{ 1.0f, 2.0f, 3.0f, 4.0f };
        float4 v2{ 5.0f, 6.0f, 7.0f, 8.0f };
        float4 v3 = v1 + v2;
        REQUIRE(v3.x == 6.0f);
        REQUIRE(v3.y == 8.0f);
        REQUIRE(v3.z == 10.0f);
        REQUIRE(v3.w == 12.0f);
    }
    SUBCASE("float3_direct_mult")
    {
        // float3 direct multiplication
        float3 v1{ 1.0f, 2.0f, 3.0f };
        float3 v2{ 4.0f, 5.0f, 6.0f };
        float3 v3 = v1 * v2; // per-element multiplication
        // expected result: { 4.0f, 10.0f, 18.0f }
        REQUIRE(v3.x == 4.0f);
        REQUIRE(v3.y == 10.0f);
        REQUIRE(v3.z == 18.0f);
    }
    SUBCASE("float3_dot_product")
    {
        // float3 dot product
        float3 v1{ 1.0f, 2.0f, 3.0f };
        float3 v2{ 4.0f, 5.0f, 6.0f };
        float  dot_product = skr::math::dot(v1, v2);
        // expected result: 32.0f (1*4 + 2*5 + 3*6)
        REQUIRE(dot_product == approx(32.0f));
    }
}

bool check_mat33(skr::float3x3& m, float m00, float m01, float m02, float m10, float m11, float m12, float m20, float m21, float m22)
{
    CHECK(m.rows[0][0] == m00);
    CHECK(m.rows[0][1] == m01);
    CHECK(m.rows[0][2] == m02);
    CHECK(m.rows[1][0] == m10);
    CHECK(m.rows[1][1] == m11);
    CHECK(m.rows[1][2] == m12);
    CHECK(m.rows[2][0] == m20);
    CHECK(m.rows[2][1] == m21);
    CHECK(m.rows[2][2] == m22);
    return true;
}

TEST_CASE("MatrixOps")
{
    using namespace skr;
    SUBCASE("float3x3_mat_ctor")
    {
        // skr matrix is row-major
        float3x3 m1{ 1.0f, 2.0f, 3.0f,
                     4.0f, 5.0f, 6.0f,
                     7.0f, 8.0f, 9.0f };
        CHECK(check_mat33(m1, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f));
    }
    SUBCASE("float3x3_matmul")
    {
        float3x3 m1{ 1.0f, 2.0f, 3.0f,
                     4.0f, 5.0f, 6.0f,
                     7.0f, 8.0f, 9.0f };
        float3x3 m2{ 9.0f, 8.0f, 7.0f,
                     6.0f, 5.0f, 4.0f,
                     3.0f, 2.0f, 1.0f };

        float3x3 m3 = m1 * m2;
        // 1 2 3  x 9 8 7 = 30 24 18
        // 4 5 6    6 5 4   84 69 54
        // 7 8 9    3 2 1   138 114 90
        CHECK(check_mat33(m3, 30.0f, 24.0f, 18.0f, 84.0f, 69.0f, 54.0f, 138.0f, 114.0f, 90.0f));
        float3x3 m4 = m2 * m1;
        // 9 8 7  x 1 2 3 = 90 114 138
        // 6 5 4    4 5 6   54 69 84
        // 3 2 1    7 8 9   18 24 30
        CHECK(check_mat33(m4, 90.0f, 114.0f, 138.0f, 54.0f, 69.0f, 84.0f, 18.0f, 24.0f, 30.0f));
    }
}

TEST_CASE("MatrixVectorOps")
{
    SUBCASE("float3x3_vector_mul")
    {
        skr::float3x3 m{ 1.0f, 2.0f, 3.0f,
                         4.0f, 5.0f, 6.0f,
                         7.0f, 8.0f, 9.0f };
        skr::float3   v{ 1.0f, 2.0f, 3.0f };
        // here we only support v * m, not m * v
        skr::float3 result = v * m;
        // 1 2 3  x 1 2 3 = 30
        //          4 5 6   36
        //          7 8 9   138
        CHECK(result.x == 30.0f);
        CHECK(result.y == 36.0f);
        CHECK(result.z == 42.0f);
    }
}