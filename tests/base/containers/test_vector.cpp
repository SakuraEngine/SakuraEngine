#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"

template <typename TestVector, typename ModifyCapacity, typename ClampCapacity, typename CheckData, typename CheckNoData, typename CheckDataEQ>
void template_test_vector(ModifyCapacity&& capacity_of, ClampCapacity&& clamp_capacity, CheckData&& check_data, CheckNoData&& check_no_data, CheckDataEQ&& check_data_eq)
{
    using skr::Greater;
    using namespace skr::test_container;

    SUBCASE("ctor")
    {
        TestVector a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        TestVector b(20);
        REQUIRE_EQ(b.size(), 20);
        REQUIRE_GE(b.capacity(), capacity_of(20));
        check_data(b);

        TestVector c(20, 114514);
        REQUIRE_EQ(c.size(), 20);
        REQUIRE_GE(c.capacity(), capacity_of(20));
        check_data(c);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(c[i], 114514);
        }

        TestVector d(c.data(), c.size() - 5);
        REQUIRE_EQ(d.size(), 15);
        REQUIRE_GE(d.capacity(), capacity_of(15));
        check_data(d);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(d[i], 114514);
        }

        TestVector e({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(e.size(), 6);
        REQUIRE_GE(e.capacity(), capacity_of(6));
        check_data(e);
        REQUIRE_EQ(e[0], 1);
        REQUIRE_EQ(e[1], 1);
        REQUIRE_EQ(e[2], 4);
        REQUIRE_EQ(e[3], 5);
        REQUIRE_EQ(e[4], 1);
        REQUIRE_EQ(e[5], 4);
    }

    SUBCASE("copy & move")
    {
        TestVector a(100, 114514);
        a.reserve(capacity_of(114514)); // test for bad capacity copy

        TestVector b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), capacity_of(a.size()));
        check_data(b);

        auto       old_size     = a.size();
        auto       old_capacity = a.capacity();
        auto       old_data     = a.data();
        TestVector c            = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        check_data_eq(c, old_data);
    }

    SUBCASE("assign & move assign")
    {
        TestVector a(100, 114514), b, c;

        b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), capacity_of(a.size()));
        check_data(b);

        auto old_size     = a.size();
        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        check_data_eq(c, old_data);
    }

    SUBCASE("spacial assign")
    {
        TestVector a(100, 114514);
        TestVector b(200, 114);

        a.assign({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_GE(a.capacity(), capacity_of(100));
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.assign(b.data(), b.size());
        REQUIRE_EQ(a.size(), 200);
        REQUIRE_GE(a.capacity(), capacity_of(200));
        for (uint32_t i = 0; i < 200; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
    }

    SUBCASE("compare")
    {
        TestVector a({ 1, 1, 4, 5, 1, 4 });
        TestVector b(6);
        TestVector c(10, 114514);
        TestVector d(6, 114514);
        b[0] = 1;
        b[1] = 1;
        b[2] = 4;
        b[3] = 5;
        b[4] = 1;
        b[5] = 4;
        REQUIRE_EQ(a.readonly(), b.readonly());
        REQUIRE_NE(b.readonly(), c.readonly());
        REQUIRE_NE(b.readonly(), d.readonly());
        REQUIRE_NE(a.readonly(), d.readonly());
        REQUIRE_NE(c.readonly(), d.readonly());
    }

    // [needn't test] getter

    SUBCASE("validate")
    {
        TestVector a(10), b;

        REQUIRE_FALSE(a.readonly().is_valid_index(-1));
        REQUIRE_FALSE(a.readonly().is_valid_index(11));
        REQUIRE(a.readonly().is_valid_index(5));
        REQUIRE(a.readonly().is_valid_index(0));
        REQUIRE(a.readonly().is_valid_index(9));

        REQUIRE_FALSE(b.readonly().is_valid_index(-1));
        REQUIRE_FALSE(b.readonly().is_valid_index(0));
        REQUIRE_FALSE(b.readonly().is_valid_index(1));
    }

    SUBCASE("memory op")
    {
        TestVector a(50, 114514);

        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data_eq(a, old_data);

        a = { 1, 1, 4, 5, 1, 4 };
        a.release(20);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(20));

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        // release eq to capcity
        a.reserve(capacity_of(4096));
        a.release(capacity_of(4096));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(4096));
        check_data(a);
        a.release();

        a.reserve(60);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(60));
        check_data(a);

        a.shrink();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        check_no_data(a);

        a = TestVector(10, 114514);
        a.resize(40, 1145140);
        REQUIRE_EQ(a.size(), 40);
        REQUIRE_GE(a.capacity(), capacity_of(40));
        check_data(a);
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }

        old_capacity = a.capacity();
        a.clear();
        a.resize_unsafe(36);
        REQUIRE_EQ(a.size(), 36);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data(a);

        a.clear();
        a.resize_default(38);
        REQUIRE_EQ(a.size(), 38);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data(a);

        a.clear();
        a.resize_zeroed(21);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
        check_data(a);
        for (uint32_t i = 0; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 0);
        }
    }

    SUBCASE("add")
    {
        TestVector a(10, 114514);
        a.add(1145140, 5);
        a.add(114514, 20);
        REQUIRE_EQ(a.size(), 35);
        REQUIRE_GE(a.capacity(), capacity_of(35));
        check_data(a);
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }
        for (uint32_t i = 15; i < 35; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        auto old_capacity = a.capacity();
        a.clear();
        a.add_unique(1);
        a.add_unique(1);
        a.add_unique(4);
        a.add_unique(5);
        a.add_unique(1);
        a.add_unique(4);
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 4);
        REQUIRE_EQ(a[2], 5);

        a.add_zeroed();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a[3], 0);

        a.add_unsafe();
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a[4], 114514);

        a.add_default();
        REQUIRE_EQ(a.size(), 6);

        a.add_unsafe(10);
        REQUIRE_EQ(a.size(), 16);

        a.add_default(10);
        REQUIRE_EQ(a.size(), 26);

        REQUIRE_EQ(a.capacity(), capacity_of(old_capacity));
    }

    SUBCASE("add at")
    {
        TestVector a(10, 114514);

        a.add_at(5, 1145140, 20);
        REQUIRE_EQ(a.size(), 30);
        for (uint32_t i = 0; i < 5; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 5; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }
        for (uint32_t i = 25; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.clear();
        a.add(10, 50);
        a.add_at_zeroed(25, 100);
        REQUIRE_EQ(a.size(), 150);
        for (uint32_t i = 0; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 10);
        }
        for (uint32_t i = 25; i < 125; ++i)
        {
            REQUIRE_EQ(a[i], 0);
        }
        for (uint32_t i = 125; i < 150; ++i)
        {
            REQUIRE_EQ(a[i], 10);
        }

        a.clear();
        a.add(114514, 30);
        a.add_at_unsafe(15, 10);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 25; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.clear();
        a.add(114514, 30);
        a.add_at_default(15, 10);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 25; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        TestVector b{};
        b.add_at(0, 1);
        b.add_at(0, 1);
        b.add_at(0, 4);
        b.add_at(0, 5);
        b.add_at(0, 1);
        b.add_at(0, 4);
        REQUIRE_EQ(b[0], 4);
        REQUIRE_EQ(b[1], 1);
        REQUIRE_EQ(b[2], 5);
        REQUIRE_EQ(b[3], 4);
        REQUIRE_EQ(b[4], 1);
        REQUIRE_EQ(b[5], 1);
    }

    SUBCASE("emplace")
    {
        TestVector a(10, 114514);
        a.emplace(10);
        REQUIRE_EQ(a.size(), 11);
        REQUIRE_GE(a.capacity(), capacity_of(11));
        check_data(a);
        REQUIRE_EQ(a[10], 10);

        a.emplace_at(5, 25);
        REQUIRE_EQ(a.size(), 12);
        REQUIRE_GE(a.capacity(), capacity_of(12));
        check_data(a);
        for (uint32_t i = 0; i < 5; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[5], 25);
        for (uint32_t i = 6; i < 11; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[11], 10);
    }

    SUBCASE("append")
    {
        TestVector a(20, 114514);
        TestVector b(10, 114);

        a.append(b);
        REQUIRE_EQ(a.size(), 30);
        REQUIRE_GE(a.capacity(), capacity_of(30));
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }

        a.append({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 36);
        REQUIRE_GE(a.capacity(), capacity_of(36));
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[30], 1);
        REQUIRE_EQ(a[31], 1);
        REQUIRE_EQ(a[32], 4);
        REQUIRE_EQ(a[33], 5);
        REQUIRE_EQ(a[34], 1);
        REQUIRE_EQ(a[35], 4);

        a.append(b.data(), 5);
        REQUIRE_EQ(a.size(), 41);
        REQUIRE_GE(a.capacity(), capacity_of(41));
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[30], 1);
        REQUIRE_EQ(a[31], 1);
        REQUIRE_EQ(a[32], 4);
        REQUIRE_EQ(a[33], 5);
        REQUIRE_EQ(a[34], 1);
        REQUIRE_EQ(a[35], 4);
        for (uint32_t i = 36; i < 41; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
    }

    SUBCASE("append at")
    {
        TestVector a(20, 114514);
        TestVector b(10, 114);

        a.append_at(10, b);
        REQUIRE_EQ(a.size(), 30);
        REQUIRE_GE(a.capacity(), capacity_of(30));
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append_at(20, { 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 36);
        REQUIRE_GE(a.capacity(), capacity_of(36));
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[20], 1);
        REQUIRE_EQ(a[21], 1);
        REQUIRE_EQ(a[22], 4);
        REQUIRE_EQ(a[23], 5);
        REQUIRE_EQ(a[24], 1);
        REQUIRE_EQ(a[25], 4);
        for (uint32_t i = 26; i < 36; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append_at(10, b.data(), 5);
        REQUIRE_EQ(a.size(), 41);
        REQUIRE_GE(a.capacity(), capacity_of(41));
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[25], 1);
        REQUIRE_EQ(a[26], 1);
        REQUIRE_EQ(a[27], 4);
        REQUIRE_EQ(a[28], 5);
        REQUIRE_EQ(a[29], 1);
        REQUIRE_EQ(a[30], 4);
        for (uint32_t i = 31; i < 41; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        TestVector c;
        c.append_at(0, { 1, 1, 4 });
        c.append_at(0, { 5, 1, 4 });
        REQUIRE_EQ(c[0], 5);
        REQUIRE_EQ(c[1], 1);
        REQUIRE_EQ(c[2], 4);
        REQUIRE_EQ(c[3], 1);
        REQUIRE_EQ(c[4], 1);
        REQUIRE_EQ(c[5], 4);
    }

    SUBCASE("remove")
    {
        TestVector a = { 1, 1, 4, 5, 1, 4 };
        a.add(114514, 20);

        a.remove_at(0, 2);
        REQUIRE_EQ(a.size(), 24);
        REQUIRE_GE(a.capacity(), capacity_of(26));
        REQUIRE_EQ(a[0], 4);
        REQUIRE_EQ(a[1], 5);
        REQUIRE_EQ(a[2], 1);
        REQUIRE_EQ(a[3], 4);
        for (uint32_t i = 4; i < 24; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.remove_at_swap(0, 2);
        REQUIRE_EQ(a.size(), 22);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 1);
        REQUIRE_EQ(a[3], 4);
        for (uint32_t i = 4; i < 22; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove(1);
        REQUIRE_EQ(a.size(), 27);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 1);
        REQUIRE_EQ(a[22], 1);
        REQUIRE_EQ(a[23], 4);
        REQUIRE_EQ(a[24], 5);
        REQUIRE_EQ(a[25], 1);
        REQUIRE_EQ(a[26], 4);

        a.remove_swap(1);
        REQUIRE_EQ(a.size(), 26);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 4);
        REQUIRE_EQ(a[22], 1);
        REQUIRE_EQ(a[23], 4);
        REQUIRE_EQ(a[24], 5);
        REQUIRE_EQ(a[25], 1);

        a.remove_last(1);
        REQUIRE_EQ(a.size(), 25);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 4);
        REQUIRE_EQ(a[22], 1);
        REQUIRE_EQ(a[23], 4);
        REQUIRE_EQ(a[24], 5);

        a.remove_last_swap(1);
        REQUIRE_EQ(a.size(), 24);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[21], 4);
        REQUIRE_EQ(a[22], 5);
        REQUIRE_EQ(a[23], 4);

        a.remove_all_swap(4);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a[0], 114514);
        REQUIRE_EQ(a[1], 114514);
        REQUIRE_EQ(a[2], 5);
        for (uint32_t i = 3; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        a.append({ 5, 5, 5 });
        a.remove_all(5);
        REQUIRE_EQ(a.size(), 20);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        TestVector aa = { 5, 1, 2, 5, 5, 2, 5 };
        aa.remove_all(5);
        REQUIRE_EQ(aa.size(), 3);
        for (uint32_t i = 0; i < aa.size(); ++i)
        {
            REQUIRE_NE(aa[i], 5);
        }
    }

    // [test in remove] remove if

    // [needn't test] modify

    SUBCASE("find")
    {
        TestVector a({ 1, 1, 4, 5, 1, 4 });

        REQUIRE_EQ(a.find(1).index(), 0);
        REQUIRE_EQ(a.find(4).index(), 2);
        REQUIRE_EQ(a.find(5).index(), 3);
        REQUIRE_EQ(a.find_last(1).index(), 4);
        REQUIRE_EQ(a.find_last(4).index(), 5);
        REQUIRE_EQ(a.find_last(5).index(), 3);

        REQUIRE_EQ(a.readonly().find(1).index(), 0);
        REQUIRE_EQ(a.readonly().find(4).index(), 2);
        REQUIRE_EQ(a.readonly().find(5).index(), 3);
        REQUIRE_EQ(a.readonly().find_last(1).index(), 4);
        REQUIRE_EQ(a.readonly().find_last(4).index(), 5);
        REQUIRE_EQ(a.readonly().find_last(5).index(), 3);
    }

    // [test in find] find if

    // [test in find] contains

    SUBCASE("sort")
    {
        const uint64_t kVectorSize = clamp_capacity(114514);

        TestVector a(kVectorSize);
        for (int i = 0; i < kVectorSize; ++i)
        {
            a[i] = kVectorSize - 1 - i;
        }

        a.sort();
        for (int i = 0; i < kVectorSize; ++i)
        {
            REQUIRE_EQ(a[i], i);
        }

        a.sort_stable(Greater<uint32_t>());
        for (int i = 0; i < kVectorSize; ++i)
        {
            REQUIRE_EQ(a[i], kVectorSize - 1 - i);
        }
    }

    SUBCASE("heap")
    {
        TestVector a({ 1, 1, 4, 5, 1, 4, 1 });

        REQUIRE_FALSE(a.is_heap());

        a.heapify();
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);

        a.heap_pop();
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 6);

        REQUIRE_EQ(a.heap_pop_get(), 1);
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 5);

        a.heap_push(100);
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 6);

        auto ref = a.find(1);
        a.heap_remove_at(ref.index());
        REQUIRE(a.is_heap());
        REQUIRE_EQ(a.heap_top(), 1);
        REQUIRE_EQ(a.size(), 5);

        a.heap_sort();
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 4);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 100);
    }

    // [test in above code] stack

    // [needn't test] support foreach

    // test cursor & iterator & foreach
    SUBCASE("Cursor & iterator")
    {
        TestVector a;
        const auto kVectorSize = clamp_capacity(100);
        a.reserve(kVectorSize);
        for (size_t i = 0; i < kVectorSize; ++i)
        {
            a.add(i);
        }

        auto test_function = [kVectorSize](auto&& arr) {
            uint64_t count;

            // iter
            count = 0;
            for (auto it = arr.iter(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.ref(), count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);
            count = 0;
            for (auto it = arr.iter_inv(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.ref(), kVectorSize - 1 - count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);

            // range
            count = 0;
            for (auto v : arr.range())
            {
                REQUIRE_EQ(v, count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);
            count = 0;
            for (auto v : arr.range_inv())
            {
                REQUIRE_EQ(v, kVectorSize - 1 - count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);

            // cursor
            count = 0;
            for (auto it = arr.cursor_begin(); !it.reach_end(); it.move_next())
            {
                REQUIRE_EQ(it.ref(), count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);
            count = 0;
            for (auto it = arr.cursor_end(); !it.reach_begin(); it.move_prev())
            {
                REQUIRE_EQ(it.ref(), kVectorSize - 1 - count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);

            // foreach
            count = 0;
            for (auto v : arr)
            {
                REQUIRE_EQ(v, count);
                ++count;
            }
            REQUIRE_EQ(count, kVectorSize);
        };

        test_function(a);
        test_function(a.readonly());
    }

    // test API when empty
    SUBCASE("empty container")
    {
        TestVector a;

        REQUIRE(a == a);
        REQUIRE_FALSE(a != a);
        REQUIRE(a.readonly() == a.readonly());
        REQUIRE_FALSE(a.readonly() != a.readonly());

        a.clear();
        a.release();
        a.shrink();
        a.resize(0, 1);
        a.resize_unsafe(0);
        a.resize_default(0);
        a.resize_zeroed(0);
        check_no_data(a);

        REQUIRE_FALSE((bool)a.remove(10));
        REQUIRE_FALSE((bool)a.remove_last(10));
        REQUIRE_FALSE((bool)a.remove_all(10));

        REQUIRE_FALSE((bool)a.find(10));
        REQUIRE_FALSE((bool)a.find_last(10));
        REQUIRE_FALSE((bool)a.readonly().find(10));
        REQUIRE_FALSE((bool)a.readonly().find_last(10));

        REQUIRE_EQ(a.contains(10), false);
        REQUIRE_EQ(a.count(10), 0);

        a.sort();
        a.sort_stable();

        a.heapify();
        REQUIRE_EQ(a.readonly().is_heap(), true);
        a.heap_sort();

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
            for ([[maybe_unused]] auto v : arr.range())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for ([[maybe_unused]] auto v : arr.range_inv())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // cursor
            count = 0;
            for (auto it = arr.cursor_begin(); !it.reach_end(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto it = arr.cursor_end(); !it.reach_begin(); it.move_prev())
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

TEST_CASE("test vector")
{
    using namespace skr::test_container;
    using TestVector = Vector<uint32_t>;

    template_test_vector<TestVector>(
        [](auto capacity) { return capacity; },
        [](auto capacity) { return capacity; },
        [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
        [](auto&& vec) { REQUIRE_EQ(vec.data(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_EQ(vec.data(), v); }
    );
}

TEST_CASE("test fixed vector")
{
    using namespace skr::test_container;
    using namespace skr::container;
    static constexpr uint64_t kFixedCapacity = 200;

    using TestVector = FixedVector<uint32_t, kFixedCapacity>;

    template_test_vector<TestVector>(
        [](auto capacity) { return kFixedCapacity; },
        [](auto capacity) { return capacity < kFixedCapacity ? capacity : kFixedCapacity; },
        [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
        [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_NE(vec.data(), nullptr); }
    );
}

TEST_CASE("test inline vector")
{
    using namespace skr::test_container;
    static constexpr uint64_t kInlineCapacity = 10;

    using TestVector = InlineVector<uint32_t, kInlineCapacity>;

    template_test_vector<TestVector>(
        [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
        [](auto capacity) { return capacity; },
        [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
        [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_NE(vec.data(), nullptr); }
    );

    SUBCASE("inline copy & move & assign & move assign")
    {
        InlineVector<uint32_t, 4> v({ 1, 2, 3, 4 });
        InlineVector<uint32_t, 4> v_copy{ v };
        InlineVector<uint32_t, 4> v_move{ std::move(v) };
        InlineVector<uint32_t, 4> v_assign;
        v_assign = v_copy;
        InlineVector<uint32_t, 4> v_move_assign;
        v_move_assign = std::move(v_move);
    }
}

TEST_CASE("test inline vector2")
{
    using namespace skr::test_container;
    static constexpr uint64_t kInlineCapacity = 0;

    using TestVector = InlineVector<uint32_t, kInlineCapacity>;

    template_test_vector<TestVector>(
        [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
        [](auto capacity) { return capacity; },
        [](auto&& vec) { REQUIRE_NE(vec.data(), nullptr); },
        [](auto&& vec) { REQUIRE_EQ(vec.data(), nullptr); },
        [](auto&& vec, auto&& v) { REQUIRE_NE(vec.data(), nullptr); }
    );
}