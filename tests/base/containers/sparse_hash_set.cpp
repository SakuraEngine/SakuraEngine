#include "SkrTestFramework/framework.hpp"
#include "skr_test_allocator.hpp"

#include "SkrBase/containers/sparse_hash_set/sparse_hash_set.hpp"

TEST_CASE("test sparse hash set")
{
    using namespace skr;
    using TestHashSet = SparseHashSet<int32_t, uint64_t, SparseHashSetConfigDefault<int32_t>, SkrTestAllocator>;

    SUBCASE("ctor & dtor") {}
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