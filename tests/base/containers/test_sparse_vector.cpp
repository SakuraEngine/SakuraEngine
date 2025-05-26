#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"

template <typename TestSparseVector, typename ModifyCapacity, typename ClampCapacity, typename CheckData, typename CheckNoData, typename CheckDataEQ>
void template_test_sparse_vector(ModifyCapacity&& capacity_of, ClampCapacity&& clamp_capacity, CheckData&& check_data, CheckNoData&& check_no_data, CheckDataEQ&& check_data_eq)
{
    using skr::Greater;
    using namespace skr::test_container;
    using TestVector = Vector<uint32_t>;

    SUBCASE("ctor")
    {
        TestSparseVector a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));

        TestSparseVector b(100);
        REQUIRE_EQ(b.size(), 100);
        REQUIRE_EQ(b.sparse_size(), 100);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(100));
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE(b.has_data(i));
        }

        TestSparseVector c(100, 114514);
        REQUIRE_EQ(c.size(), 100);
        REQUIRE_EQ(c.sparse_size(), 100);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), capacity_of(100));
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE(c.has_data(i));
            REQUIRE_EQ(c[i], 114514);
        }

        TestSparseVector d({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(d.size(), 6);
        REQUIRE_EQ(d.sparse_size(), 6);
        REQUIRE_EQ(d.hole_size(), 0);
        REQUIRE_GE(d.capacity(), capacity_of(6));
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

        uint32_t         data[] = { 1, 1, 4, 5, 1, 4 };
        TestSparseVector e(data, 6);
        REQUIRE_EQ(e.size(), 6);
        REQUIRE_EQ(e.sparse_size(), 6);
        REQUIRE_EQ(e.hole_size(), 0);
        REQUIRE_GE(e.capacity(), capacity_of(6));
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
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        a.reserve(capacity_of(114514)); // test for bad capacity copy
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        TestSparseVector b(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(6));
        REQUIRE_EQ(b[0], 1);
        REQUIRE_EQ(b[1], 1);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        REQUIRE_EQ(b[4], 1);
        REQUIRE_EQ(b[5], 4);

        auto             old_capacity = a.capacity();
        TestSparseVector c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        REQUIRE_EQ(c[0], 1);
        REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);
    }

    SUBCASE("copy & move(with hole)")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE_EQ(a[0], 1);
        // REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        // REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        TestSparseVector b(a);
        REQUIRE_EQ(b.size(), 4);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 2);
        REQUIRE_GE(b.capacity(), capacity_of(6));
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
        REQUIRE_GE(b.capacity(), capacity_of(6));
        REQUIRE_EQ(b[0], 1);
        REQUIRE_EQ(b[1], 2);
        REQUIRE_EQ(b[2], 4);
        REQUIRE_EQ(b[3], 5);
        REQUIRE_EQ(b[4], 2);
        REQUIRE_EQ(b[5], 4);

        auto             old_capacity = a.capacity();
        TestSparseVector c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 4);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 2);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
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
        REQUIRE_GE(c.capacity(), capacity_of(7));
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
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        TestSparseVector b({ 114514, 114514, 1, 1, 4 });
        TestSparseVector c({ 1, 1, 4, 514, 514, 514 });

        b = a;
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(6));
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
        REQUIRE_GE(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        REQUIRE_EQ(c[0], 1);
        REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 5);
        REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);
    }

    SUBCASE("assign & move assign(with hole)")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        TestSparseVector b({ 114514, 114514, 1, 1, 4 });
        TestSparseVector c({ 1, 1, 4, 514, 514, 514 });
        a.remove_at(1);
        a.remove_at(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), capacity_of(6));
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
        REQUIRE_GE(b.capacity(), capacity_of(6));
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
        REQUIRE_GE(b.capacity(), capacity_of(6));
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
        REQUIRE_GE(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 4);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 2);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
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
        REQUIRE_GE(c.capacity(), capacity_of(7));
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
        uint32_t data[100];
        for (size_t i = 0; i < 100; ++i)
        {
            data[i] = static_cast<uint32_t>(99 - i);
        }

        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        a.assign(data, 50);
        REQUIRE_EQ(a.size(), 50);
        REQUIRE_EQ(a.sparse_size(), 50);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(50));
        for (size_t i = 0; i < 50; ++i)
        {
            a[i] = static_cast<uint32_t>(99 - i);
        }

        a.assign({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(50));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("compare")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        TestSparseVector b({ 114, 114, 514, 114, 514, 114 });
        TestSparseVector c({ 1, 1, 4, 5, 1, 4 });

        REQUIRE_EQ(a.readonly(), c.readonly());
        REQUIRE_NE(a.readonly(), b.readonly());

        c.remove_at(1);
        REQUIRE_NE(a.readonly(), c.readonly());

        a.remove_at(1);
        REQUIRE_EQ(a.readonly(), c.readonly());
    }

    // [needn't test] getter

    SUBCASE("validate")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
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
    }

    SUBCASE("memory op")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(5);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), capacity_of(6));

        auto old_capacity = a.capacity();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        // release eq to capcity
        a.reserve(capacity_of(4096));
        a.release(capacity_of(4096));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(4096));
        check_data(a);
        a.release();

        a.release(5);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(5));

        a.reserve(100);
        a.append({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(100));

        a.remove_at(4);
        a.remove_at(5);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(4));
        a.release();

        a.clear();
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(3);
        a.compact();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
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
        REQUIRE_GE(a.capacity(), capacity_of(6));
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
        REQUIRE_GE(a.capacity(), capacity_of(12));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
        a.compact_top();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), capacity_of(12));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
    }

    SUBCASE("add")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        auto             info = a.add(10);
        info.ref()            = 100;
        REQUIRE_EQ(a.size(), 7);
        REQUIRE_EQ(a.sparse_size(), 7);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(7));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 100);

        const auto add_count = clamp_capacity(114514 + 7) - 7;

        for (int32_t i = 0; i < add_count; ++i)
        {
            a.add(i);
        }
        for (int32_t i = 0; i < add_count; ++i)
        {
            REQUIRE_EQ(a[i + 7], i);
        }

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
        info.ref() = 114514;
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
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
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
        TestSparseVector a({ { 1, 1, 4, 5, 1, 4 } });
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
        TestSparseVector a;
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove_at(1);
        a.remove_at(4);
        a.append({ 114514, 114514, 114514 });
        REQUIRE_EQ(a.size(), 7);
        REQUIRE_EQ(a.sparse_size(), 7);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(7));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 114514);
        REQUIRE_EQ(a[5], 4);
        REQUIRE_EQ(a[6], 114514);

        TestSparseVector b;
        b.append({ 1, 1, 4, 5, 1, 4 });
        b.remove_at(0);
        b.remove_at(1);
        b.remove_at(5);
        a.append(b);
        REQUIRE_EQ(a.size(), 10);
        REQUIRE_EQ(a.sparse_size(), 10);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(10));
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

        TestVector c(100, 114514);
        a.remove_at(6, 4);
        a.append(c.data(), c.size());
        REQUIRE_EQ(a.size(), 106);
        REQUIRE_EQ(a.sparse_size(), 106);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(106));
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
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
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
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });
        a.remove_if([](const uint32_t& a) { return a > 3; });
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

        a.remove_last_if([](const uint32_t& a) { return a > 3; });
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

        a.remove_all_if([](const uint32_t& a) { return a < 3; });
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

    SUBCASE("contains & count")
    {
        TestSparseVector a({ 1, 1, 4, 5, 1, 4 });

        REQUIRE(a.contains(5));
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);
        a.remove_all(5);
        REQUIRE_FALSE(a.contains(5));
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 0);

        auto cond = [](const uint32_t& a) { return a < 4; };
        REQUIRE(a.contains_if(cond));
        REQUIRE_EQ(a.count_if(cond), 3);
        a.remove_all_if(cond);
        REQUIRE_FALSE(a.contains_if(cond));
        REQUIRE_EQ(a.count_if(cond), 0);
    }

    SUBCASE("sort")
    {
        const auto kCapacity    = clamp_capacity(114514);
        const auto kRemovedSize = kCapacity / 2;

        TestSparseVector a(kCapacity);
        for (size_t i = 0; i < kCapacity; ++i)
        {
            a[i] = static_cast<uint32_t>(kCapacity - 1 - i);
        }
        a.remove_all_if([](const uint32_t& n) { return n % 2 == 1; });
        REQUIRE_EQ(a.size(), kRemovedSize);
        REQUIRE_EQ(a.sparse_size(), kCapacity);
        REQUIRE_EQ(a.hole_size(), kCapacity - kRemovedSize);
        REQUIRE_EQ(a.capacity(), capacity_of(kCapacity));
        a.sort();
        REQUIRE_EQ(a.size(), kRemovedSize);
        REQUIRE_EQ(a.sparse_size(), kRemovedSize);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(kCapacity));
        for (size_t i = 0; i < kRemovedSize; ++i)
        {
            REQUIRE_EQ(a[i], i * 2);
        }

        a.clear();
        for (size_t i = 0; i < kCapacity; ++i)
        {
            a.add(static_cast<uint32_t>(i));
        }
        a.remove_all_if([](const uint32_t& n) { return n % 2 == 1; });
        REQUIRE_EQ(a.size(), kRemovedSize);
        REQUIRE_EQ(a.sparse_size(), kCapacity);
        REQUIRE_EQ(a.hole_size(), kCapacity - kRemovedSize);
        REQUIRE_EQ(a.capacity(), capacity_of(kCapacity));
        a.sort_stable(Greater<uint32_t>());
        REQUIRE_EQ(a.size(), kRemovedSize);
        REQUIRE_EQ(a.sparse_size(), kRemovedSize);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(kCapacity));
        for (size_t i = 0; i < kRemovedSize; ++i)
        {
            REQUIRE_EQ(a[i], (kRemovedSize - 1 - i) * 2);
        }
    }

    SUBCASE("cursor & iter")
    {
        const auto kCapacity = clamp_capacity(114514);

        TestSparseVector a(kCapacity);
        for (size_t i = 0; i < kCapacity; ++i)
        {
            a[i] = i;
        }
        for (size_t i = 0; i < kCapacity; ++i)
        {
            if (i % 2 == 1)
            {
                a.remove_at(i);
            }
        }

        auto test_func = [kCapacity](auto&& arr) {
            uint64_t count;

            // iter
            count = 0;
            for (auto it = arr.iter(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.ref(), count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto it = arr.iter_inv(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.ref(), (kCapacity / 2 - count - 1) * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // range
            count = 0;
            for (auto n : arr.range())
            {
                REQUIRE_EQ(n, count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto n : arr.range_inv())
            {
                REQUIRE_EQ(n, (kCapacity / 2 - count - 1) * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // cursor
            count = 0;
            for (auto cur = arr.cursor_begin(); !cur.reach_end(); cur.move_next())
            {
                REQUIRE_EQ(cur.ref(), count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto cur = arr.cursor_end(); !cur.reach_begin(); cur.move_prev())
            {
                REQUIRE_EQ(cur.ref(), (kCapacity / 2 - count - 1) * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // foreach
            count = 0;
            for (auto v : arr)
            {
                REQUIRE_EQ(v, count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
        };

        test_func(a);
        test_func(a.readonly());
    }

    SUBCASE("empty container")
    {
        TestSparseVector a;

        REQUIRE(a == a);
        REQUIRE_FALSE(a != a);
        REQUIRE(a.readonly() == a.readonly());
        REQUIRE_FALSE(a.readonly() != a.readonly());

        a.clear();
        a.release();
        a.reserve(0);
        a.shrink();
        a.compact();
        a.compact_stable();
        a.compact_top();
        check_no_data(a);

        REQUIRE_FALSE(a.remove(114514));
        REQUIRE_FALSE(a.remove_last(114514));
        REQUIRE_EQ(a.remove_all(114514), 0);

        REQUIRE_FALSE((bool)a.find(114514));
        REQUIRE_FALSE((bool)a.find_last(114514));
        REQUIRE_FALSE((bool)a.readonly().find(114514));
        REQUIRE_FALSE((bool)a.readonly().find_last(114514));

        REQUIRE_FALSE(a.contains(114514));
        REQUIRE_FALSE(a.readonly().contains(114514));
        REQUIRE_EQ(a.count(114514), 0);
        REQUIRE_EQ(a.readonly().count(114514), 0);

        a.sort();
        a.sort_stable();

        auto test_func = [](auto&& arr) {
            uint64_t count;

            // iter
            count = 0;
            for (auto it = arr.iter(); it.has_next(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto it = arr.iter_inv(); it.has_next(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // range
            count = 0;
            for ([[maybe_unused]] auto n : arr.range())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for ([[maybe_unused]] auto n : arr.range_inv())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // cursor
            count = 0;
            for (auto cur = arr.cursor_begin(); !cur.reach_end(); cur.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto cur = arr.cursor_end(); !cur.reach_begin(); cur.move_prev())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // foreach
            count = 0;
            for ([[maybe_unused]] auto v : arr)
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
        };
        test_func(a);
        test_func(a.readonly());
    }
}

TEST_CASE("test sparse vector")
{
    using namespace skr::test_container;
    using TestSparseVector = SparseVector<uint32_t>;

    template_test_sparse_vector<TestSparseVector>(
        [](auto capacity) { return capacity; },
        [](auto capacity) { return capacity; },
        [](auto&& vec) { REQUIRE_NE(vec.storage(), nullptr); },
        [](auto&& vec) { REQUIRE_EQ(vec.storage(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_EQ(vec.storage(), v); }
    );
}

TEST_CASE("test fixed sparse vector")
{
    using namespace skr::test_container;
    static constexpr uint64_t kFixedCapacity = 200;

    using TestSparseVector = FixedSparseVector<uint32_t, kFixedCapacity>;

    template_test_sparse_vector<TestSparseVector>(
        [](auto capacity) { return kFixedCapacity; },
        [](auto capacity) { return capacity < kFixedCapacity ? capacity : kFixedCapacity; },
        [](auto&& vec) { REQUIRE_NE(vec.storage(), nullptr); },
        [](auto&& vec) { REQUIRE_NE(vec.storage(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_NE(vec.storage(), nullptr); }
    );
}

TEST_CASE("test inline sparse vector")
{
    using namespace skr::test_container;

    static constexpr uint64_t kInlineCapacity = 10;

    using TestSparseVector = InlineSparseVector<uint32_t, kInlineCapacity>;

    template_test_sparse_vector<TestSparseVector>(
        [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
        [](auto capacity) { return capacity; },
        [](auto&& vec) { REQUIRE_NE(vec.storage(), nullptr); },
        [](auto&& vec) { REQUIRE_NE(vec.storage(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_NE(vec.storage(), nullptr); }
    );

    SUBCASE("inline copy & move & assign & move assign")
    {
        InlineSparseVector<uint32_t, 4> v({ 1, 2, 3, 4 });

        InlineSparseVector<uint32_t, 4> v_copy{ v };
        InlineSparseVector<uint32_t, 4> v_move{ std::move(v) };
        InlineSparseVector<uint32_t, 4> v_assign;
        v_assign = v_copy;
        InlineSparseVector<uint32_t, 4> v_move_assign;
        v_move_assign = std::move(v_move);
    }
}
