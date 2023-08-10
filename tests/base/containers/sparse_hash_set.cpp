#include "SkrTestFramework/framework.hpp"
#include "skr_test_allocator.hpp"

#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"

TEST_CASE("test sparse hash set (Single)")
{
    using namespace skr;
    using TestHashSet = SparseHashSet<int32_t, uint64_t, SparseHashSetConfigDefault<int32_t>, SkrTestAllocator>;

    SUBCASE("ctor & dtor")
    {
        TestHashSet a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.bucket_size(), 0);
        REQUIRE_EQ(a.data().data(), nullptr);

        TestHashSet b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 100);
        REQUIRE_EQ(b.bucket_size(), 0);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_FALSE(b.has_data(i));
        }

        TestHashSet c({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE_GE(c.bucket_size(), 4);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));

        int32_t     data[] = { 1, 1, 4, 5, 1, 4 };
        TestHashSet d(data, 6);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE_GE(c.bucket_size(), 4);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));
    }

    SUBCASE("copy & move") {}
    SUBCASE("assign & move assign") {}
    SUBCASE("compare") {}

    // [needn't test] getter
    // [needn't test] validate

    SUBCASE("memory op") {}

    // [needn't test] data op
    // [needn't test] bucket op

    SUBCASE("add") {}
    SUBCASE("emplace") {}
    SUBCASE("append") {}
    SUBCASE("remove") {}
    SUBCASE("find") {}
    SUBCASE("contain") {}
    SUBCASE("sort") {}
    SUBCASE("set ops") {}
}