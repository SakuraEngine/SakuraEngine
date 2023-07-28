#include <gtest/gtest.h>
#include "SkrRT/stl/array/array.hpp"

TEST(TestSTL, test_array)
{
    using namespace skr;

    // ctor
    {
        Array<uint32_t> a;
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), 0);
        ASSERT_EQ(a.data(), nullptr);

        Array<uint32_t> b(20);
        ASSERT_EQ(b.size(), 20);
        ASSERT_GE(b.capacity(), 20);
        ASSERT_NE(b.data(), nullptr);

        Array<uint32_t> c(20, 114514);
        ASSERT_EQ(c.size(), 20);
        ASSERT_GE(c.capacity(), 20);
        ASSERT_NE(c.data(), nullptr);
        for (uint32_t i = 0; i < 20; ++i)
        {
            ASSERT_EQ(c[i], 114514);
        }

        Array<uint32_t> d(c.data(), c.size() - 5);
        ASSERT_EQ(d.size(), 15);
        ASSERT_GE(d.capacity(), 15);
        ASSERT_NE(d.data(), nullptr);
        for (uint32_t i = 0; i < 15; ++i)
        {
            ASSERT_EQ(d[i], 114514);
        }

        Array<uint32_t> e({ 1, 1, 4, 5, 1, 4 });
        ASSERT_EQ(e.size(), 6);
        ASSERT_GE(e.capacity(), 6);
        ASSERT_NE(e.data(), nullptr);
        ASSERT_EQ(e[0], 1);
        ASSERT_EQ(e[1], 1);
        ASSERT_EQ(e[2], 4);
        ASSERT_EQ(e[3], 5);
        ASSERT_EQ(e[4], 1);
        ASSERT_EQ(e[5], 4);
    }

    // copy & move
    {
        Array<uint32_t> a(100, 114514);

        Array<uint32_t> b = a;
        ASSERT_EQ(b.size(), a.size());
        ASSERT_GE(b.capacity(), a.size());
        ASSERT_NE(b.data(), nullptr);

        auto            old_size     = a.size();
        auto            old_capacity = a.capacity();
        auto            old_data     = a.data();
        Array<uint32_t> c            = std::move(a);
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), 0);
        ASSERT_EQ(a.data(), nullptr);
        ASSERT_EQ(c.size(), old_size);
        ASSERT_EQ(c.capacity(), old_capacity);
        ASSERT_EQ(c.data(), old_data);
    }

    // assign & move assign
    {
        Array<uint32_t> a(100, 114514), b, c;

        b = a;
        ASSERT_EQ(b.size(), a.size());
        ASSERT_GE(b.capacity(), a.size());
        ASSERT_NE(b.data(), nullptr);

        auto old_size     = a.size();
        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        c                 = std::move(a);
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), 0);
        ASSERT_EQ(a.data(), nullptr);
        ASSERT_EQ(c.size(), old_size);
        ASSERT_EQ(c.capacity(), old_capacity);
        ASSERT_EQ(c.data(), old_data);
    }

    // spacial assign
    {
        Array<uint32_t> a(100, 114514);
        Array<uint32_t> b(200, 114);

        a.assign({ 1, 1, 4, 5, 1, 4 });
        ASSERT_EQ(a.size(), 6);
        ASSERT_GE(a.capacity(), 100);
        ASSERT_EQ(a[0], 1);
        ASSERT_EQ(a[1], 1);
        ASSERT_EQ(a[2], 4);
        ASSERT_EQ(a[3], 5);
        ASSERT_EQ(a[4], 1);
        ASSERT_EQ(a[5], 4);

        a.assign(b.data(), b.size());
        ASSERT_EQ(a.size(), 200);
        ASSERT_GE(a.capacity(), 200);
        for (uint32_t i = 0; i < 200; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
    }

    // compare
    {
        Array<uint32_t> a({ 1, 1, 4, 5, 1, 4 });
        Array<uint32_t> b(6);
        Array<uint32_t> c(10, 114514);
        Array<uint32_t> d(6, 114514);
        b[0] = 1;
        b[1] = 1;
        b[2] = 4;
        b[3] = 5;
        b[4] = 1;
        b[5] = 4;
        ASSERT_EQ(a, b);
        ASSERT_NE(b, c);
        ASSERT_NE(b, d);
        ASSERT_NE(a, d);
        ASSERT_NE(c, d);
    }

    // [needn't test] getter

    // validate
    {
        Array<uint32_t> a(10), b;

        ASSERT_FALSE(a.is_valid_index(-1));
        ASSERT_FALSE(a.is_valid_index(11));
        ASSERT_TRUE(a.is_valid_index(5));
        ASSERT_TRUE(a.is_valid_index(0));
        ASSERT_TRUE(a.is_valid_index(9));

        ASSERT_FALSE(b.is_valid_index(-1));
        ASSERT_FALSE(b.is_valid_index(0));
        ASSERT_FALSE(b.is_valid_index(1));

        ASSERT_TRUE(a.is_valid_pointer(a.begin()));
        ASSERT_TRUE(a.is_valid_pointer(a.begin() + 5));
        ASSERT_TRUE(a.is_valid_pointer(a.end() - 1));
        ASSERT_FALSE(a.is_valid_pointer(a.begin() - 1));
        ASSERT_FALSE(a.is_valid_pointer(a.end()));
    }

    // memory op
    {
        Array<uint32_t> a(50, 114514);

        auto old_capacity = a.capacity();
        auto old_data     = a.data();
        a.clear();
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), old_capacity);
        ASSERT_EQ(a.data(), old_data);

        a = { 1, 1, 4, 5, 1, 4 };
        a.release(20);
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), 20);

        a.release();
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), 0);
        ASSERT_EQ(a.data(), nullptr);

        a.reserve(60);
        ASSERT_EQ(a.size(), 0);
        ASSERT_GE(a.capacity(), 60);
        ASSERT_NE(a.data(), nullptr);

        a.shrink();
        ASSERT_EQ(a.size(), 0);
        ASSERT_EQ(a.capacity(), 0);
        ASSERT_EQ(a.data(), nullptr);

        a = Array<uint32_t>(10, 114514);
        a.resize(40, 1145140);
        ASSERT_EQ(a.size(), 40);
        ASSERT_GE(a.capacity(), 40);
        ASSERT_NE(a.data(), nullptr);
        for (uint32_t i = 0; i < 10; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 40; ++i)
        {
            ASSERT_EQ(a[i], 1145140);
        }

        old_capacity = a.capacity();
        a.clear();
        a.resize_unsafe(36);
        ASSERT_EQ(a.size(), 36);
        ASSERT_EQ(a.capacity(), old_capacity);
        ASSERT_NE(a.data(), nullptr);

        a.clear();
        a.resize_default(38);
        ASSERT_EQ(a.size(), 38);
        ASSERT_EQ(a.capacity(), old_capacity);
        ASSERT_NE(a.data(), nullptr);

        a.clear();
        a.resize_zeroed(21);
        ASSERT_EQ(a.size(), 21);
        ASSERT_EQ(a.capacity(), old_capacity);
        ASSERT_NE(a.data(), nullptr);
        for (uint32_t i = 0; i < 21; ++i)
        {
            ASSERT_EQ(a[i], 0);
        }
    }

    // add
    {
        Array<uint32_t> a(10, 114514);
        a.add(1145140, 5);
        a.add(114514, 20);
        ASSERT_EQ(a.size(), 35);
        ASSERT_GE(a.capacity(), 35);
        ASSERT_NE(a.data(), nullptr);
        for (uint32_t i = 0; i < 10; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 15; ++i)
        {
            ASSERT_EQ(a[i], 1145140);
        }
        for (uint32_t i = 15; i < 35; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        auto old_capacity = a.capacity();
        a.clear();
        a.add_unique(1);
        a.add_unique(1);
        a.add_unique(4);
        a.add_unique(5);
        a.add_unique(1);
        a.add_unique(4);
        ASSERT_EQ(a.size(), 3);
        ASSERT_EQ(a[0], 1);
        ASSERT_EQ(a[1], 4);
        ASSERT_EQ(a[2], 5);

        a.add_zeroed();
        ASSERT_EQ(a.size(), 4);
        ASSERT_EQ(a[3], 0);

        a.add_unsafe();
        ASSERT_EQ(a.size(), 5);
        ASSERT_EQ(a[4], 114514);

        a.add_default();
        ASSERT_EQ(a.size(), 6);

        a.add_unsafe(10);
        ASSERT_EQ(a.size(), 16);

        a.add_default(10);
        ASSERT_EQ(a.size(), 26);

        ASSERT_EQ(a.capacity(), old_capacity);
    }

    // add at
    {
        Array<uint32_t> a(10, 114514);

        a.add_at(5, 1145140, 20);
        ASSERT_EQ(a.size(), 30);
        for (uint32_t i = 0; i < 5; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 5; i < 25; ++i)
        {
            ASSERT_EQ(a[i], 1145140);
        }
        for (uint32_t i = 25; i < 30; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.clear();
        a.add(10, 50);
        a.add_at_zeroed(25, 100);
        ASSERT_EQ(a.size(), 150);
        for (uint32_t i = 0; i < 25; ++i)
        {
            ASSERT_EQ(a[i], 10);
        }
        for (uint32_t i = 25; i < 125; ++i)
        {
            ASSERT_EQ(a[i], 0);
        }
        for (uint32_t i = 125; i < 150; ++i)
        {
            ASSERT_EQ(a[i], 10);
        }

        a.clear();
        a.add(114514, 30);
        a.add_at_unsafe(15, 10);
        for (uint32_t i = 0; i < 15; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 25; i < 40; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.clear();
        a.add(114514, 30);
        a.add_at_default(15, 10);
        for (uint32_t i = 0; i < 15; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 25; i < 40; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
    }

    // emplace
    {
        Array<uint32_t> a(10, 114514);
        a.emplace(10);
        ASSERT_EQ(a.size(), 11);
        ASSERT_GE(a.capacity(), 11);
        ASSERT_NE(a.data(), nullptr);
        ASSERT_EQ(a[10], 10);

        a.emplace_at(5, 25);
        ASSERT_EQ(a.size(), 12);
        ASSERT_GE(a.capacity(), 12);
        ASSERT_NE(a.data(), nullptr);
        for (uint32_t i = 0; i < 5; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        ASSERT_EQ(a[5], 25);
        for (uint32_t i = 6; i < 11; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        ASSERT_EQ(a[11], 10);
    }

    // append
    {
        Array<uint32_t> a(20, 114514);
        Array<uint32_t> b(10, 114);

        a.append(b);
        ASSERT_EQ(a.size(), 30);
        ASSERT_GE(a.capacity(), 30);
        for (uint32_t i = 0; i < 20; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }

        a.append({ 1, 1, 4, 5, 1, 4 });
        ASSERT_EQ(a.size(), 36);
        ASSERT_GE(a.capacity(), 36);
        for (uint32_t i = 0; i < 20; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
        ASSERT_EQ(a[30], 1);
        ASSERT_EQ(a[31], 1);
        ASSERT_EQ(a[32], 4);
        ASSERT_EQ(a[33], 5);
        ASSERT_EQ(a[34], 1);
        ASSERT_EQ(a[35], 4);

        a.append(b.data(), 5);
        ASSERT_EQ(a.size(), 41);
        ASSERT_GE(a.capacity(), 41);
        for (uint32_t i = 0; i < 20; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
        ASSERT_EQ(a[30], 1);
        ASSERT_EQ(a[31], 1);
        ASSERT_EQ(a[32], 4);
        ASSERT_EQ(a[33], 5);
        ASSERT_EQ(a[34], 1);
        ASSERT_EQ(a[35], 4);
        for (uint32_t i = 36; i < 41; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
    }

    // append at
    {
        Array<uint32_t> a(20, 114514);
        Array<uint32_t> b(10, 114);

        a.append_at(10, b);
        ASSERT_EQ(a.size(), 30);
        ASSERT_GE(a.capacity(), 30);
        for (uint32_t i = 0; i < 10; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 20; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
        for (uint32_t i = 20; i < 30; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.append_at(20, { 1, 1, 4, 5, 1, 4 });
        ASSERT_EQ(a.size(), 36);
        ASSERT_GE(a.capacity(), 36);
        for (uint32_t i = 0; i < 10; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 20; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
        ASSERT_EQ(a[20], 1);
        ASSERT_EQ(a[21], 1);
        ASSERT_EQ(a[22], 4);
        ASSERT_EQ(a[23], 5);
        ASSERT_EQ(a[24], 1);
        ASSERT_EQ(a[25], 4);
        for (uint32_t i = 26; i < 36; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.append_at(10, b.data(), 5);
        ASSERT_EQ(a.size(), 41);
        ASSERT_GE(a.capacity(), 41);
        for (uint32_t i = 0; i < 10; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        for (uint32_t i = 10; i < 25; ++i)
        {
            ASSERT_EQ(a[i], 114);
        }
        ASSERT_EQ(a[25], 1);
        ASSERT_EQ(a[26], 1);
        ASSERT_EQ(a[27], 4);
        ASSERT_EQ(a[28], 5);
        ASSERT_EQ(a[29], 1);
        ASSERT_EQ(a[30], 4);
        for (uint32_t i = 31; i < 41; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
    }

    // remove
    {
        Array<uint32_t> a = { 1, 1, 4, 5, 1, 4 };
        a.add(114514, 20);

        a.remove_at(0, 2);
        ASSERT_EQ(a.size(), 24);
        ASSERT_GE(a.capacity(), 26);
        ASSERT_EQ(a[0], 4);
        ASSERT_EQ(a[1], 5);
        ASSERT_EQ(a[2], 1);
        ASSERT_EQ(a[3], 4);
        for (uint32_t i = 4; i < 24; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.remove_at_swap(0, 2);
        ASSERT_EQ(a.size(), 22);
        ASSERT_EQ(a[0], 114514);
        ASSERT_EQ(a[1], 114514);
        ASSERT_EQ(a[2], 1);
        ASSERT_EQ(a[3], 4);
        for (uint32_t i = 4; i < 22; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.append({ 1, 1, 4, 5, 1, 4 });
        a.remove(1);
        ASSERT_EQ(a.size(), 27);
        ASSERT_EQ(a[0], 114514);
        ASSERT_EQ(a[1], 114514);
        ASSERT_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        ASSERT_EQ(a[21], 1);
        ASSERT_EQ(a[22], 1);
        ASSERT_EQ(a[23], 4);
        ASSERT_EQ(a[24], 5);
        ASSERT_EQ(a[25], 1);
        ASSERT_EQ(a[26], 4);

        a.remove_swap(1);
        ASSERT_EQ(a.size(), 26);
        ASSERT_EQ(a[0], 114514);
        ASSERT_EQ(a[1], 114514);
        ASSERT_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        ASSERT_EQ(a[21], 4);
        ASSERT_EQ(a[22], 1);
        ASSERT_EQ(a[23], 4);
        ASSERT_EQ(a[24], 5);
        ASSERT_EQ(a[25], 1);

        a.remove_last(1);
        ASSERT_EQ(a.size(), 25);
        ASSERT_EQ(a[0], 114514);
        ASSERT_EQ(a[1], 114514);
        ASSERT_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        ASSERT_EQ(a[21], 4);
        ASSERT_EQ(a[22], 1);
        ASSERT_EQ(a[23], 4);
        ASSERT_EQ(a[24], 5);

        a.remove_last_swap(1);
        ASSERT_EQ(a.size(), 24);
        ASSERT_EQ(a[0], 114514);
        ASSERT_EQ(a[1], 114514);
        ASSERT_EQ(a[2], 4);
        for (uint32_t i = 3; i < 21; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
        ASSERT_EQ(a[21], 4);
        ASSERT_EQ(a[22], 5);
        ASSERT_EQ(a[23], 4);

        a.remove_all_swap(4);
        ASSERT_EQ(a.size(), 21);
        ASSERT_EQ(a[0], 114514);
        ASSERT_EQ(a[1], 114514);
        ASSERT_EQ(a[2], 5);
        for (uint32_t i = 3; i < 21; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }

        a.append({ 5, 5, 5 });
        a.remove_all(5);
        ASSERT_EQ(a.size(), 20);
        for (uint32_t i = 0; i < 20; ++i)
        {
            ASSERT_EQ(a[i], 114514);
        }
    }

    // [test in remove] remove if

    // [needn't test] modify

    // find
    {
        Array<uint32_t> a({ 1, 1, 4, 5, 1, 4 });

        ASSERT_EQ(a.find(1).index, 0);
        ASSERT_EQ(a.find(4).index, 2);
        ASSERT_EQ(a.find(5).index, 3);
        ASSERT_EQ(a.find(114514).data, nullptr);
        ASSERT_EQ(a.find_last(1).index, 4);
        ASSERT_EQ(a.find_last(4).index, 5);
        ASSERT_EQ(a.find_last(5).index, 3);
        ASSERT_EQ(a.find_last(114514).data, nullptr);
    }

    // [test in find] find if

    // [test in find] contain

    // sort
    {
        Array<uint32_t> a(100);
        for (int i = 0; i < 100; ++i)
        {
            a[i] = 99 - i;
        }

        a.sort();
        for (int i = 0; i < 100; ++i)
        {
            ASSERT_EQ(a[i], i);
        }

        a.sort_stable(Greater<uint32_t>());
        for (int i = 0; i < 100; ++i)
        {
            ASSERT_EQ(a[i], 99 - i);
        }
    }

    // heap
    {
        Array<uint32_t> a({ 1, 1, 4, 5, 1, 4, 1 });

        ASSERT_FALSE(a.is_heap());

        a.heapify();
        ASSERT_TRUE(a.is_heap());
        ASSERT_EQ(a.heap_top(), 1);

        a.heap_pop();
        ASSERT_TRUE(a.is_heap());
        ASSERT_EQ(a.heap_top(), 1);
        ASSERT_EQ(a.size(), 6);

        ASSERT_EQ(a.heap_pop_get(), 1);
        ASSERT_TRUE(a.is_heap());
        ASSERT_EQ(a.heap_top(), 1);
        ASSERT_EQ(a.size(), 5);

        a.heap_push(100);
        ASSERT_TRUE(a.is_heap());
        ASSERT_EQ(a.heap_top(), 1);
        ASSERT_EQ(a.size(), 6);

        auto ref = a.find(1);
        a.heap_remove_at(ref.index);
        ASSERT_TRUE(a.is_heap());
        ASSERT_EQ(a.heap_top(), 1);
        ASSERT_EQ(a.size(), 5);

        a.heap_sort();
        ASSERT_EQ(a[0], 1);
        ASSERT_EQ(a[1], 4);
        ASSERT_EQ(a[2], 4);
        ASSERT_EQ(a[3], 5);
        ASSERT_EQ(a[4], 100);
    }

    // [test in above code] stack

    // [needn't test] support foreach
}
