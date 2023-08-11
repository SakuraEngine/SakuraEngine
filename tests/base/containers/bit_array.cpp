#include "SkrTestFramework/framework.hpp"

#include "SkrBase/containers/bit_array/bit_array.hpp"
#include "skr_test_allocator.hpp"

TEST_CASE("test bit array")
{
    using namespace skr;
    using BitArray = skr::BitArray<uint64_t, skr::SkrTestAllocator>;

    SUBCASE("test ctor")
    {
        BitArray a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);
        REQUIRE(a.empty());

        BitArray b(20, true);
        REQUIRE_EQ(b.size(), 20);
        REQUIRE_GE(b.capacity(), 20);
        REQUIRE_NE(b.data(), nullptr);
        for (int i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(b[i], true);
        }

        BitArray c(100, false);
        REQUIRE_EQ(c.size(), 100);
        REQUIRE_GE(c.capacity(), 100);
        REQUIRE_NE(c.data(), nullptr);
        for (int i = 0; i < 100; ++i)
        {
            REQUIRE_EQ(c[i], false);
        }
    }

    SUBCASE("test copy & move")
    {
        BitArray a(30, true);
        for (int i = 0; i < 30; ++i)
        {
            a[i] = i % 2;
        }

        BitArray b(a);
        REQUIRE_EQ(a.size(), b.size());
        REQUIRE_EQ(a.capacity(), b.capacity());
        for (int i = 0; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], b[i]);
        }

        auto     old_data = a.data();
        BitArray c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);
        REQUIRE_EQ(c.size(), b.size());
        REQUIRE_EQ(c.capacity(), b.capacity());
        REQUIRE_EQ(c.data(), old_data);
        for (int i = 0; i < 30; ++i)
        {
            REQUIRE_EQ(b[i], c[i]);
        }
    }

    SUBCASE("test copy assign & move assign")
    {
        BitArray a(30, true), b, c;

        b = a;
        REQUIRE_EQ(a.size(), b.size());
        for (int i = 0; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], b[i]);
        }

        auto old_data = a.data();
        c             = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);
        REQUIRE_EQ(c.size(), b.size());
        REQUIRE_EQ(c.capacity(), b.capacity());
        REQUIRE_EQ(c.data(), old_data);
        for (int i = 0; i < 30; ++i)
        {
            REQUIRE_EQ(b[i], c[i]);
        }
    }

    SUBCASE("test compare")
    {
        BitArray a(30, true), b(30, true), c(20, true);

        REQUIRE_EQ(a, b);
        REQUIRE_NE(a, c);
    }

    // [needn't test] getter

    SUBCASE("test validate")
    {
        BitArray a(30, true);
        REQUIRE(a.is_valid_index(0));
        REQUIRE(a.is_valid_index(15));
        REQUIRE(a.is_valid_index(29));
        REQUIRE_FALSE(a.is_valid_index(-1));
        REQUIRE_FALSE(a.is_valid_index(39));
    }

    SUBCASE("test memory op")
    {
        BitArray a(30, true);
        auto     old_capacity = a.capacity();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), old_capacity);
        REQUIRE_NE(a.data(), nullptr);

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data(), nullptr);

        a.reserve(40);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_GE(a.capacity(), 40);
        REQUIRE_NE(a.data(), nullptr);

        a.resize_unsafe(10);
        REQUIRE_EQ(a.size(), 10);

        a.clear();
        a.resize(20, true);
        REQUIRE_EQ(a.size(), 20);
        for (int i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], true);
        }

        a.resize_unsafe(512);
        REQUIRE_GE(a.capacity(), 512);

        a.reserve(4096);
        REQUIRE_GE(a.capacity(), 4096);

        a.release(64);
        REQUIRE_GE(a.capacity(), 64);

        a.resize_unsafe(64);
        a.resize_unsafe(16);
        REQUIRE_EQ(a.size(), 16);
    }

    SUBCASE("test add")
    {
        BitArray a;

        for (int i = 0; i < 40; ++i)
        {
            a.add(i % 3 == 0);
        }
        REQUIRE_EQ(a.size(), 40);
        REQUIRE_GE(a.capacity(), 40);
        for (int i = 0; i < 40; ++i)
        {
            REQUIRE_EQ(a[i], i % 3 == 0);
        }

        a.add(true, 10);
        REQUIRE_EQ(a.size(), 50);
        REQUIRE_GE(a.capacity(), 50);
        for (int i = 40; i < 50; ++i)
        {
            REQUIRE_EQ(a[i], true);
        }
    }

    SUBCASE("test remove")
    {
        BitArray a;
        a.add(true, 10);
        a.add(false, 20);
        a.add(true, 10);
        a.remove_at(10, 20);
        REQUIRE_EQ(a.size(), 20);
        for (int i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], true);
        }

        a.add(false, 15);
        a.remove_at_swap(15, 5);
        REQUIRE_EQ(a.size(), 30);
        for (int i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(a[i], true);
        }
        for (int i = 15; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], false);
        }

        a.add(true, 5);
        a.remove_at_swap(20, 10);
        REQUIRE_EQ(a.size(), 25);
        for (int i = 15; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], false);
        }
        for (int i = 20; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], true);
        }
    }

    SUBCASE("test modify")
    {
        BitArray a(30, true);

        for (int i = 0; i < 30; ++i)
        {
            if (i % 3 == 0)
            {
                a[i] = false;
            }
        }
        REQUIRE_EQ(a.size(), 30);

        for (int i = 0; i < 30; ++i)
        {
            REQUIRE_EQ(a[i], i % 3 != 0);
        }
        REQUIRE_EQ(a.size(), 30);
        for (int i = 0; i < 30; ++i)
        {
            if (i % 3 == 0)
            {
                a[i] = true;
            }
        }
        REQUIRE_EQ(a.size(), 30);
        for (auto v : a)
        {
            REQUIRE_EQ(v, true);
        }
        REQUIRE_EQ(a.size(), 30);
        for (auto v : a)
        {
            v = false;
        }
        REQUIRE_EQ(a.size(), 30);
        for (bool v : a)
        {
            REQUIRE_EQ(v, false);
        }
        REQUIRE_EQ(a.size(), 30);
    }

    SUBCASE("test find")
    {
        BitArray a(30, true), b(30, false);

        REQUIRE_EQ(a.find(false), npos_of<size_t>);
        REQUIRE_EQ(b.find(true), npos_of<size_t>);

        a[20] = a[10] = false;
        b[20] = b[10] = true;

        REQUIRE_EQ(a.find(false), 10);
        REQUIRE_EQ(a.find_last(false), 20);
        REQUIRE_EQ(b.find(true), 10);
        REQUIRE_EQ(b.find_last(true), 20);
    }

    // [included in above tests] setRange()

    SUBCASE("test true it")
    {
        BitArray a;
        a.reserve(30);
        for (int i = 0; i < 30; ++i)
        {
            a.add(i % 5 == 0);
        }

        for (BitArray::TIt it(a.data(), a.size()); it; ++it)
        {
            REQUIRE(it.index() % 5 == 0);
        }
    }
}