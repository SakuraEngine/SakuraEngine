#include "SkrTestFramework/framework.hpp"

#include "SkrBase/containers/array/array.hpp"
#include "skr_test_allocator.hpp"

TEST_CASE("test array")
{
    using namespace skr;
    using namespace skr::container;
    using TestArray = Array<uint32_t, SkrTestAllocator>;

    SUBCASE("ctor")
    {
        TestArray a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);

        TestArray b(20);
        REQUIRE_EQ(b.size(), 20);
        REQUIRE_GE(b.capacity(), 20);
        REQUIRE_NE(b.data(), nullptr);

        TestArray c(20, 114514);
        REQUIRE_EQ(c.size(), 20);
        REQUIRE_GE(c.capacity(), 20);
        REQUIRE_NE(c.data(), nullptr);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(c[i], 114514);
        }

        TestArray d(c.data(), c.size() - 5);
        REQUIRE_EQ(d.size(), 15);
        REQUIRE_GE(d.capacity(), 15);
        REQUIRE_NE(d.data(), nullptr);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(d[i], 114514);
        }

        TestArray e({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(e.size(), 6);
        REQUIRE_GE(e.capacity(), 6);
        REQUIRE_NE(e.data(), nullptr);
        REQUIRE_EQ(e[0], 1);
        REQUIRE_EQ(e[1], 1);
        REQUIRE_EQ(e[2], 4);
        REQUIRE_EQ(e[3], 5);
        REQUIRE_EQ(e[4], 1);
        REQUIRE_EQ(e[5], 4);
    }

    SUBCASE("copy & move")
    {
        TestArray a(100, 114514);

        TestArray b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), a.size());
        REQUIRE_NE(b.data(), nullptr);

        auto      old_size     = a.size();
        auto      old_capacity = a.capacity();
        auto      old_data     = a.data();
        TestArray c            = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_EQ(c.data(), old_data);
    }

    SUBCASE("assign & move assign")
    {
        TestArray a(100, 114514), b, c;

        b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), a.size());
        REQUIRE_NE(b.data(), nullptr);

        auto old_size     = a.size();
        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_EQ(c.data(), old_data);
    }

    SUBCASE("spacial assign")
    {
        TestArray a(100, 114514);
        TestArray b(200, 114);

        a.assign({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_GE(a.capacity(), 100);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.assign(b.data(), b.size());
        REQUIRE_EQ(a.size(), 200);
        REQUIRE_GE(a.capacity(), 200);
        for (uint32_t i = 0; i < 200; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
    }

    SUBCASE("compare")
    {
        TestArray a({ 1, 1, 4, 5, 1, 4 });
        TestArray b(6);
        TestArray c(10, 114514);
        TestArray d(6, 114514);
        b[0] = 1;
        b[1] = 1;
        b[2] = 4;
        b[3] = 5;
        b[4] = 1;
        b[5] = 4;
        REQUIRE_EQ(a, b);
        REQUIRE_NE(b, c);
        REQUIRE_NE(b, d);
        REQUIRE_NE(a, d);
        REQUIRE_NE(c, d);
    }

    // [needn't test] getter

    SUBCASE("validate")
    {
        TestArray a(10), b;

        REQUIRE_FALSE(a.is_valid_index(-1));
        REQUIRE_FALSE(a.is_valid_index(11));
        REQUIRE(a.is_valid_index(5));
        REQUIRE(a.is_valid_index(0));
        REQUIRE(a.is_valid_index(9));

        REQUIRE_FALSE(b.is_valid_index(-1));
        REQUIRE_FALSE(b.is_valid_index(0));
        REQUIRE_FALSE(b.is_valid_index(1));

        REQUIRE(a.is_valid_pointer(a.begin()));
        REQUIRE(a.is_valid_pointer(a.begin() + 5));
        REQUIRE(a.is_valid_pointer(a.end() - 1));
        REQUIRE_FALSE(a.is_valid_pointer(a.begin() - 1));
        REQUIRE_FALSE(a.is_valid_pointer(a.end()));
    }

    SUBCASE("memory op")
    {
        TestArray a(50, 114514);

        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), old_capacity);
        REQUIRE_EQ(a.data(), old_data);

        a = { 1, 1, 4, 5, 1, 4 };
        a.release(20);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 20);

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);

        a.reserve(60);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_GE(a.capacity(), 60);
        REQUIRE_NE(a.data(), nullptr);

        a.shrink();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);

        a = TestArray(10, 114514);
        a.resize(40, 1145140);
        REQUIRE_EQ(a.size(), 40);
        REQUIRE_GE(a.capacity(), 40);
        REQUIRE_NE(a.data(), nullptr);
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
        REQUIRE_EQ(a.capacity(), old_capacity);
        REQUIRE_NE(a.data(), nullptr);

        a.clear();
        a.resize_default(38);
        REQUIRE_EQ(a.size(), 38);
        REQUIRE_EQ(a.capacity(), old_capacity);
        REQUIRE_NE(a.data(), nullptr);

        a.clear();
        a.resize_zeroed(21);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a.capacity(), old_capacity);
        REQUIRE_NE(a.data(), nullptr);
        for (uint32_t i = 0; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 0);
        }
    }

    SUBCASE("add")
    {
        TestArray a(10, 114514);
        a.add(1145140, 5);
        a.add(114514, 20);
        REQUIRE_EQ(a.size(), 35);
        REQUIRE_GE(a.capacity(), 35);
        REQUIRE_NE(a.data(), nullptr);
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

        REQUIRE_EQ(a.capacity(), old_capacity);
    }

    SUBCASE("add at")
    {
        TestArray a(10, 114514);

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
    }

    SUBCASE("emplace")
    {
        TestArray a(10, 114514);
        a.emplace(10);
        REQUIRE_EQ(a.size(), 11);
        REQUIRE_GE(a.capacity(), 11);
        REQUIRE_NE(a.data(), nullptr);
        REQUIRE_EQ(a[10], 10);

        a.emplace_at(5, 25);
        REQUIRE_EQ(a.size(), 12);
        REQUIRE_GE(a.capacity(), 12);
        REQUIRE_NE(a.data(), nullptr);
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
        TestArray a(20, 114514);
        TestArray b(10, 114);

        a.append(b);
        REQUIRE_EQ(a.size(), 30);
        REQUIRE_GE(a.capacity(), 30);
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
        REQUIRE_GE(a.capacity(), 36);
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
        REQUIRE_GE(a.capacity(), 41);
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
        TestArray a(20, 114514);
        TestArray b(10, 114);

        a.append_at(10, b);
        REQUIRE_EQ(a.size(), 30);
        REQUIRE_GE(a.capacity(), 30);
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
        REQUIRE_GE(a.capacity(), 36);
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
        REQUIRE_GE(a.capacity(), 41);
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
    }

    SUBCASE("remove")
    {
        TestArray a = { 1, 1, 4, 5, 1, 4 };
        a.add(114514, 20);

        a.remove_at(0, 2);
        REQUIRE_EQ(a.size(), 24);
        REQUIRE_GE(a.capacity(), 26);
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

        TestArray aa = {5, 1, 2, 5, 5, 2, 5};
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
        TestArray a({ 1, 1, 4, 5, 1, 4 });

        REQUIRE_EQ(a.find(1).index, 0);
        REQUIRE_EQ(a.find(4).index, 2);
        REQUIRE_EQ(a.find(5).index, 3);
        REQUIRE_EQ(a.find(114514).data, nullptr);
        REQUIRE_EQ(a.find_last(1).index, 4);
        REQUIRE_EQ(a.find_last(4).index, 5);
        REQUIRE_EQ(a.find_last(5).index, 3);
        REQUIRE_EQ(a.find_last(114514).data, nullptr);
    }

    // [test in find] find if

    // [test in find] contain

    SUBCASE("sort")
    {
        TestArray a(100);
        for (int i = 0; i < 100; ++i)
        {
            a[i] = 99 - i;
        }

        a.sort();
        for (int i = 0; i < 100; ++i)
        {
            REQUIRE_EQ(a[i], i);
        }

        a.sort_stable(Greater<uint32_t>());
        for (int i = 0; i < 100; ++i)
        {
            REQUIRE_EQ(a[i], 99 - i);
        }
    }

    SUBCASE("heap")
    {
        TestArray a({ 1, 1, 4, 5, 1, 4, 1 });

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
        a.heap_remove_at(ref.index);
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

    // test iterator
    SUBCASE("iterator")
    {
        TestArray a;
        for (auto n : a)
        {
            printf("%d\n", n);
        }
    }
}