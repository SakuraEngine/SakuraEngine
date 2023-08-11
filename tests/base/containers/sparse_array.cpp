#include "SkrTestFramework/framework.hpp"
#include "skr_test_allocator.hpp"

#include "SkrBase/containers/sparse_array/sparse_array.hpp"
#include "SkrBase/containers/array/array.hpp"

TEST_CASE("test sparse array")
{
    using namespace skr;
    using TestSparseArray = SparseArray<uint32_t, uint64_t, SkrTestAllocator>;
    using TestArray       = Array<uint32_t, SkrTestAllocator>;

    SUBCASE("ctor")
    {
        TestSparseArray a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);

        TestSparseArray b(100);
        REQUIRE_EQ(b.size(), 100);
        REQUIRE_EQ(b.sparse_size(), 100);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 100);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE(b.has_data(i));
        }

        TestSparseArray c(100, 114514);
        REQUIRE_EQ(c.size(), 100);
        REQUIRE_EQ(c.sparse_size(), 100);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 100);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE(c.has_data(i));
            REQUIRE_EQ(c[i], 114514);
        }

        TestSparseArray d({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(d.size(), 6);
        REQUIRE_EQ(d.sparse_size(), 6);
        REQUIRE_EQ(d.hole_size(), 0);
        REQUIRE_GE(d.capacity(), 6);
        REQUIRE(d.has_data(0));
        REQUIRE_EQ(d[0], 1);
        REQUIRE(d.has_data(1));
        REQUIRE_EQ(d[1], 1);
        REQUIRE(d.has_data(2));
        REQUIRE_EQ(d[2], 4);
        REQUIRE(d.has_data(3));
        REQUIRE_EQ(d[3], 5);
        REQUIRE(d.has_data(4));
        REQUIRE_EQ(d[4], 1);
        REQUIRE(d.has_data(5));
        REQUIRE_EQ(d[5], 4);

        u32             data[] = { 1, 1, 4, 5, 1, 4 };
        TestSparseArray e(data, 6);
        REQUIRE_EQ(e.size(), 6);
        REQUIRE_EQ(e.sparse_size(), 6);
        REQUIRE_EQ(e.hole_size(), 0);
        REQUIRE_GE(e.capacity(), 6);
        REQUIRE(e.has_data(0));
        REQUIRE_EQ(e[0], 1);
        REQUIRE(e.has_data(1));
        REQUIRE_EQ(e[1], 1);
        REQUIRE(e.has_data(2));
        REQUIRE_EQ(e[2], 4);
        REQUIRE(e.has_data(3));
        REQUIRE_EQ(e[3], 5);
        REQUIRE(e.has_data(4));
        REQUIRE_EQ(e[4], 1);
        REQUIRE(e.has_data(5));
        REQUIRE_EQ(e[5], 4);
    }

    SUBCASE("copy & move")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        TestSparseArray b(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_EQ(b[0], 1);
        REQUIRE_EQ(b[1], 1);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        REQUIRE_EQ(b[4], 1);
        REQUIRE_EQ(b[5], 4);

        auto            old_capacity = a.capacity();
        TestSparseArray c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 0);
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_EQ(c[0], 1);
        REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);
    }

    SUBCASE("copy & move(with hole)")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        TestSparseArray b(a);
        REQUIRE_EQ(b.size(), 4);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 2);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_EQ(b[0], 1);
        // REQUIRE_EQ(b[1], 1);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(b[5], 4);

        b.append({ 2, 2 });
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_EQ(b[0], 1);
        REQUIRE_EQ(b[1], 2);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        REQUIRE_EQ(b[4], 2);
        REQUIRE_EQ(b[5], 4);

        auto            old_capacity = a.capacity();
        TestSparseArray c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 0);
        REQUIRE_EQ(c.size(), 4);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 2);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_EQ(c[0], 1);
        // REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        // REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);

        c.append({ 2, 2, 1 });
        REQUIRE_EQ(c.size(), 7);
        REQUIRE_EQ(c.sparse_size(), 7);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 7);
        REQUIRE_EQ(c[0], 1);
        REQUIRE_EQ(c[1], 2);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        REQUIRE_EQ(c[4], 2);
        REQUIRE_EQ(c[5], 4);
        REQUIRE_EQ(c[6], 1);
    }

    SUBCASE("assign & move assign")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        TestSparseArray b({ 114514, 114514, 1, 1, 4 });
        TestSparseArray c({ 1, 1, 4, 514, 514, 514 });

        b = a;
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_EQ(b[0], 1);
        REQUIRE_EQ(b[1], 1);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        REQUIRE_EQ(b[4], 1);
        REQUIRE_EQ(b[5], 4);

        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 0);
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_EQ(c[0], 1);
        REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);
    }

    SUBCASE("assign & move assign(with hole)")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        TestSparseArray b({ 114514, 114514, 1, 1, 4 });
        TestSparseArray c({ 1, 1, 4, 514, 514, 514 });
        a.remove_at(1);
        a.remove_at(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        b = a;
        REQUIRE_EQ(b.size(), 4);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 2);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_EQ(b[0], 1);
        // REQUIRE_EQ(b[1], 1);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(b[5], 4);

        b.append({ 2, 2 });
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_EQ(b[0], 1);
        REQUIRE_EQ(b[1], 2);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        REQUIRE_EQ(b[4], 2);
        REQUIRE_EQ(b[5], 4);

        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 0);
        REQUIRE_EQ(c.size(), 4);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 2);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_EQ(c[0], 1);
        // REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        // REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);

        c.append({ 2, 2, 1 });
        REQUIRE_EQ(c.size(), 7);
        REQUIRE_EQ(c.sparse_size(), 7);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 7);
        REQUIRE_EQ(c[0], 1);
        REQUIRE_EQ(c[1], 2);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        REQUIRE_EQ(c[4], 2);
        REQUIRE_EQ(c[5], 4);
        REQUIRE_EQ(c[6], 1);
    }

    SUBCASE("special assign")
    {
        u32 data[100];
        for (size_t i = 0; i < 100; ++i)
        {
            data[i] = static_cast<u32>(99 - i);
        }

        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        a.assign(data, 50);
        REQUIRE_EQ(a.size(), 50);
        REQUIRE_EQ(a.sparse_size(), 50);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 50);
        for (size_t i = 0; i < 50; ++i)
        {
            a[i] = static_cast<u32>(99 - i);
        }

        a.assign({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 50);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("compare")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        TestSparseArray b({ 114, 114, 514, 114, 514, 114 });
        TestSparseArray c({ 1, 1, 4, 5, 1, 4 });

        REQUIRE_EQ(a, c);
        REQUIRE_NE(a, b);

        c.remove_at(1);
        REQUIRE_NE(a, c);

        a.remove_at(1);
        REQUIRE_EQ(a, c);
    }

    // [needn't test] getter

    SUBCASE("validate")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(0);
        a.remove_at(2);
        a.remove_at(4);
        for (size_t i = 0; i < 6; ++i)
        {
            REQUIRE_NE(a.has_data(i), a.is_hole(i));
        }
        REQUIRE(a.is_hole(0));
        REQUIRE(a.has_data(1));
        REQUIRE(a.is_hole(2));
        REQUIRE(a.has_data(3));
        REQUIRE(a.is_hole(4));
        REQUIRE(a.has_data(5));

        REQUIRE_FALSE(a.is_valid_index(-1));
        REQUIRE(a.is_valid_index(0));
        REQUIRE(a.is_valid_index(3));
        REQUIRE(a.is_valid_index(5));
        REQUIRE_FALSE(a.is_valid_index(6));

        REQUIRE_FALSE(a.is_valid_pointer(&a[0] - 1));
        REQUIRE(a.is_valid_pointer(&a[0]));
        REQUIRE(a.is_valid_pointer(&a[3]));
        REQUIRE(a.is_valid_pointer(&a[5]));
        REQUIRE_FALSE(a.is_valid_pointer(&a[5] + 4));
    }

    SUBCASE("memory op")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(5);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 6);

        auto old_capacity = a.capacity();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), old_capacity);

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);

        a.release(5);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 5);

        a.reserve(100);
        a.append({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 100);

        a.remove_at(4);
        a.remove_at(5);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_EQ(a.capacity(), 6);

        a.clear();
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(3);
        a.compact();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 4);

        a.clear();
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(3);
        a.compact_stable();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 4);
        REQUIRE_EQ(a[2], 1);
        REQUIRE_EQ(a[3], 4);

        a.clear();
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(3);
        a.remove_at(6, 6);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 12);
        REQUIRE_EQ(a.hole_size(), 8);
        REQUIRE_GE(a.capacity(), 12);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
        a.compact_top();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 12);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("add")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        auto            info = a.add(10);
        *info.data           = 100;
        REQUIRE_EQ(a.size(), 7);
        REQUIRE_EQ(a.sparse_size(), 7);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 7);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 100);

        a.remove_at(1);
        a.add_zeroed();
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 0);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 100);

        a.remove_at(1);
        a.remove_at(4);
        info       = a.add_unsafe();
        *info.data = 114514;
        REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 0);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 114514);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 100);
    }

    SUBCASE("add at")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        a.add_at(1, 114514);
        a.add_at(4, 10086);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 10086);
        REQUIRE_EQ(a[5], 4);

        a.remove_at(2);
        a.remove_at(5);
        a.add_at_zeroed(2);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 0);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 10086);
        // REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("emplace")
    {
        TestSparseArray a({ { 1, 1, 4, 5, 1, 4 } });
        a.remove_at(1);
        a.remove_at(4);
        a.emplace(114514);
        a.emplace_at(1, 10086);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 10086);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 114514);
        REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("append")
    {
        TestSparseArray a;
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        a.append({ 114514, 114514, 114514 });
        REQUIRE_EQ(a.size(), 7);
        REQUIRE_EQ(a.sparse_size(), 7);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 7);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 114514);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 114514);

        TestSparseArray b;
        b.append({ 1, 1, 4, 5, 1, 4 });
        b.remove_at(0);
        b.remove_at(1);
        b.remove_at(5);
        a.append(b);
        REQUIRE_EQ(a.size(), 10);
        REQUIRE_EQ(a.sparse_size(), 10);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 10);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 114514);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 114514);
        REQUIRE_EQ(a[7], 4);
        REQUIRE_EQ(a[8], 5);
        REQUIRE_EQ(a[9], 1);

        TestArray c(100, 114514);
        a.remove_at(6, 4);
        a.append(c.data(), c.size());
        REQUIRE_EQ(a.size(), 106);
        REQUIRE_EQ(a.sparse_size(), 106);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 106);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 114514);
        REQUIRE_EQ(a[5], 4);
        for (size_t i = 6; i < 106; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
    }

    SUBCASE("remove")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE(a.has_data(0));
        REQUIRE(a.is_hole(1));
        REQUIRE(a.has_data(2));
        REQUIRE(a.has_data(3));
        REQUIRE(a.is_hole(4));
        REQUIRE(a.has_data(5));
        REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.remove(4);
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 3);
        REQUIRE(a.has_data(0));
        REQUIRE(a.is_hole(1));
        REQUIRE(a.is_hole(2));
        REQUIRE(a.has_data(3));
        REQUIRE(a.is_hole(4));
        REQUIRE(a.has_data(5));
        REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 1);
        // REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.clear();
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(3);
        a.remove_last(1);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE(a.has_data(0));
        REQUIRE(a.has_data(1));
        REQUIRE(a.has_data(2));
        REQUIRE(a.is_hole(3));
        REQUIRE(a.is_hole(4));
        REQUIRE(a.has_data(5));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        // REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.add(4);
        a.add(4);
        a.remove_all(4);
        REQUIRE_EQ(a.size(), 2);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 4);
        REQUIRE(a.has_data(0));
        REQUIRE(a.has_data(1));
        REQUIRE(a.is_hole(2));
        REQUIRE(a.is_hole(3));
        REQUIRE(a.is_hole(4));
        REQUIRE(a.is_hole(5));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        // REQUIRE_EQ(a[2], 4);
        // REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        // REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("remove if")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });
        a.remove_if([](const u32& a) { return a > 3; });
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 1);
        REQUIRE(a.has_data(0));
        REQUIRE(a.has_data(1));
        REQUIRE(a.is_hole(2));
        REQUIRE(a.has_data(3));
        REQUIRE(a.has_data(4));
        REQUIRE(a.has_data(5));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        // REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.remove_last_if([](const u32& a) { return a > 3; });
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE(a.has_data(0));
        REQUIRE(a.has_data(1));
        REQUIRE(a.is_hole(2));
        REQUIRE(a.has_data(3));
        REQUIRE(a.has_data(4));
        REQUIRE(a.is_hole(5));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        // REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        // REQUIRE_EQ(a[5], 4);

        a.remove_all_if([](const u32& a) { return a < 3; });
        REQUIRE_EQ(a.size(), 1);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 5);
        REQUIRE(a.is_hole(0));
        REQUIRE(a.is_hole(1));
        REQUIRE(a.is_hole(2));
        REQUIRE(a.has_data(3));
        REQUIRE(a.is_hole(4));
        REQUIRE(a.is_hole(5));
        // REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 1);
        // REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        // REQUIRE_EQ(a[5], 4);
    }

    // [needn't test] modify

    // [test in remove] find

    // [test in remove] find if

    SUBCASE("contain")
    {
        TestSparseArray a({ 1, 1, 4, 5, 1, 4 });

        REQUIRE(a.contain(5));
        a.remove_all(5);
        REQUIRE_FALSE(a.contain(5));

        auto cond = [](const u32& a) { return a < 4; };
        REQUIRE(a.contain_if(cond));
        a.remove_all_if(cond);
        REQUIRE_FALSE(a.contain_if(cond));
    }

    SUBCASE("sort")
    {
        TestSparseArray a(1000);
        for (size_t i = 0; i < 1000; ++i)
        {
            a[i] = static_cast<u32>(999 - i);
        }
        a.remove_all_if([](const u32& n) { return n % 2 == 1; });
        REQUIRE_EQ(a.size(), 500);
        REQUIRE_EQ(a.sparse_size(), 1000);
        REQUIRE_EQ(a.hole_size(), 500);
        REQUIRE_EQ(a.capacity(), 1000);
        a.sort();
        REQUIRE_EQ(a.size(), 500);
        REQUIRE_EQ(a.sparse_size(), 500);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 1000);
        for (size_t i = 0; i < 500; ++i)
        {
            REQUIRE_EQ(a[i], i * 2);
        }

        a.clear();
        for (size_t i = 0; i < 1000; ++i)
        {
            a.add(static_cast<u32>(i));
        }
        a.remove_all_if([](const u32& n) { return n % 2 == 1; });
        REQUIRE_EQ(a.size(), 500);
        REQUIRE_EQ(a.sparse_size(), 1000);
        REQUIRE_EQ(a.hole_size(), 500);
        REQUIRE_EQ(a.capacity(), 1000);
        a.sort_stable(Greater<u32>());
        REQUIRE_EQ(a.size(), 500);
        REQUIRE_EQ(a.sparse_size(), 500);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 1000);
        for (size_t i = 0; i < 500; ++i)
        {
            REQUIRE_EQ(a[i], (499 - i) * 2);
        }
    }

    SUBCASE("foreach")
    {
        TestSparseArray a(1000);
        for (size_t i = 0; i < 1000; ++i)
        {
            a[i] = static_cast<u32>(999 - i);
        }

        size_t count = 0;
        for (u32 v : a)
        {
            REQUIRE_LT(v, 1000);
            REQUIRE_GE(v, 0);
            REQUIRE_EQ(v, 999 - count);
            ++count;
        }
        REQUIRE_EQ(count, 1000);

        count = 0;
        for (u32 v : a)
        {
            REQUIRE_LT(v, 1000);
            REQUIRE_GE(v, 0);
            REQUIRE_EQ(v, 999 - count);
            ++count;
        }
        REQUIRE_EQ(count, 1000);
    }
}