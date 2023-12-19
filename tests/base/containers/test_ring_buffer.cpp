#include "SkrTestFramework/framework.hpp"
#include "container_test_types.hpp"
#include <chrono>

TEST_CASE("test ring buffer")
{
    using namespace skr;
    using TestRingBuffer = RingBuffer<uint32_t>;

    srand(std::chrono::system_clock::now().time_since_epoch().count());
    auto shuffle_ring_buffer = [](TestRingBuffer& buffer) {
        for (uint32_t i = 0; i < 114514; ++i)
        {
            auto times = rand() % 10;

            if (i % 2 == 0)
            {
                for (uint32_t j = 0; j < times; ++j)
                {
                    if (j % 2 == 0)
                    {
                        buffer.push_front(rand());
                    }
                    else
                    {
                        if (!buffer.empty())
                        {
                            buffer.pop_back();
                        }
                    }
                }
            }
            else
            {
                for (uint32_t j = 0; j < times; ++j)
                {
                    if (j % 2 == 0)
                    {
                        buffer.push_back(rand());
                    }
                    else
                    {
                        if (!buffer.empty())
                        {
                            buffer.pop_front();
                        }
                    }
                }
            }
        }
    };

    SUBCASE("ctor")
    {
        TestRingBuffer a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);

        TestRingBuffer b(20);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_GE(b.capacity(), 20);

        // TestRingBuffer c(20, 114514);
        // REQUIRE_EQ(c.size(), 20);
        // REQUIRE_GE(c.capacity(), 20);
        // for (uint32_t i = 0; i < 20; ++i)
        // {
        //     REQUIRE_EQ(c[i], 114514);
        // }

        uint32_t buffer[20];
        for (uint32_t i = 0; i < 20; ++i)
        {
            buffer[i] = 114514;
        }

        TestRingBuffer d(buffer, 15);
        REQUIRE_EQ(d.size(), 15);
        REQUIRE_GE(d.capacity(), 15);
        for (uint32_t i = 0; i < 15; ++i)
        {
            REQUIRE_EQ(d[i], 114514);
        }

        TestRingBuffer e({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(e.size(), 6);
        REQUIRE_GE(e.capacity(), 6);
        REQUIRE_EQ(e[0], 1);
        REQUIRE_EQ(e[1], 1);
        REQUIRE_EQ(e[2], 4);
        REQUIRE_EQ(e[3], 5);
        REQUIRE_EQ(e[4], 1);
        REQUIRE_EQ(e[5], 4);
    }

    SUBCASE("copy & move")
    {
        TestRingBuffer a;
        a.resize(100, 114514);

        TestRingBuffer b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), a.size());

        auto           old_size     = a.size();
        auto           old_capacity = a.capacity();
        TestRingBuffer c            = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), old_capacity);
    }

    SUBCASE("assign & move assign")
    {
        TestRingBuffer a, b, c;
        a.resize(100, 114514);

        b = a;
        REQUIRE_EQ(b.size(), a.size());
        REQUIRE_GE(b.capacity(), a.size());

        auto old_size     = a.size();
        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(c.size(), old_size);
        REQUIRE_EQ(c.capacity(), old_capacity);
    }

    SUBCASE("spacial assign")
    {
        TestRingBuffer a(100);
        uint32_t       buffer[200];
        for (uint32_t i = 0; i < 100; ++i)
        {
            a.push_back(114514);
        }
        for (uint32_t i = 0; i < 200; ++i)
        {
            buffer[i] = 114;
        }

        a.assign({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_GE(a.capacity(), 100);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        a.assign(buffer, 200);
        REQUIRE_EQ(a.size(), 200);
        REQUIRE_GE(a.capacity(), 200);
        for (uint32_t i = 0; i < 200; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
    }

    // [needn't test] getter

    SUBCASE("validate")
    {
        TestRingBuffer a(10), b;
        a.resize(10, 114514);

        REQUIRE_FALSE(a.is_valid_index(-1));
        REQUIRE_FALSE(a.is_valid_index(11));
        REQUIRE(a.is_valid_index(5));
        REQUIRE(a.is_valid_index(0));
        REQUIRE(a.is_valid_index(9));

        REQUIRE_FALSE(b.is_valid_index(-1));
        REQUIRE_FALSE(b.is_valid_index(0));
        REQUIRE_FALSE(b.is_valid_index(1));

        // REQUIRE(a.is_valid_pointer(a.begin()));
        // REQUIRE(a.is_valid_pointer(a.begin() + 5));
        // REQUIRE(a.is_valid_pointer(a.end() - 1));
        // REQUIRE_FALSE(a.is_valid_pointer(a.begin() - 1));
        // REQUIRE_FALSE(a.is_valid_pointer(a.end()));
    }

    SUBCASE("memory op")
    {
        TestRingBuffer a;
        a.resize(10, 114514);
        shuffle_ring_buffer(a);

        auto old_capacity = a.capacity();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), old_capacity);

        a = { 1, 1, 4, 5, 1, 4 };
        a.release(20);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 20);

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);

        a.reserve(60);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_GE(a.capacity(), 60);

        a.shrink();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.capacity(), 0);

        a.resize(10, 114514);
        a.resize(40, 1145140);
        REQUIRE_EQ(a.size(), 40);
        REQUIRE_GE(a.capacity(), 40);
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

        a.clear();
        a.resize_default(38);
        REQUIRE_EQ(a.size(), 38);
        REQUIRE_EQ(a.capacity(), old_capacity);

        a.clear();
        a.resize_zeroed(21);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a.capacity(), old_capacity);
        for (uint32_t i = 0; i < 21; ++i)
        {
            REQUIRE_EQ(a[i], 0);
        }
    }

    SUBCASE("add")
    {
        TestRingBuffer a;
        shuffle_ring_buffer(a);
        while (!a.empty())
        {
            a.pop_front();
        }

        a.resize(10, 114514);
        a.push_back(1145140, 5);
        a.push_back(114514, 20);
        REQUIRE_EQ(a.size(), 35);
        REQUIRE_GE(a.capacity(), 35);
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
        a.push_front(114514, 3);

        a.push_back_zeroed();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a[3], 0);

        a.push_back_unsafe();
        REQUIRE_EQ(a.size(), 5);

        a.push_back_default();
        REQUIRE_EQ(a.size(), 6);

        a.push_back_unsafe(10);
        REQUIRE_EQ(a.size(), 16);

        a.push_back_default(10);
        REQUIRE_EQ(a.size(), 26);

        REQUIRE_EQ(a.capacity(), old_capacity);

        a.release();
        a.resize(10, 114514);
        a.push_front(1145140, 5);
        a.push_front(114514, 20);
        REQUIRE_EQ(a.size(), 35);
        REQUIRE_GE(a.capacity(), 35);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        for (uint32_t i = 20; i < 25; ++i)
        {
            REQUIRE_EQ(a[i], 1145140);
        }
        for (uint32_t i = 25; i < 35; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }

        old_capacity = a.capacity();
        a.clear();
        a.push_back(114514, 3);

        a.push_front_zeroed();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a[0], 0);

        a.push_front_unsafe();
        REQUIRE_EQ(a.size(), 5);

        a.push_front_default();
        REQUIRE_EQ(a.size(), 6);

        a.push_front_unsafe(10);
        REQUIRE_EQ(a.size(), 16);

        a.push_front_default(10);
        REQUIRE_EQ(a.size(), 26);

        REQUIRE_EQ(a.capacity(), old_capacity);
    }

    SUBCASE("emplace")
    {
        TestRingBuffer a;
        a.resize(10, 114514);
        a.emplace_back(10);
        REQUIRE_EQ(a.size(), 11);
        REQUIRE_GE(a.capacity(), 11);
        REQUIRE_EQ(a[10], 10);

        a.emplace_front(25);
        REQUIRE_EQ(a.size(), 12);
        REQUIRE_GE(a.capacity(), 12);
        REQUIRE_EQ(a[0], 25);
        for (uint32_t i = 0; i < 10; ++i)
        {
            REQUIRE_EQ(a[i + 1], 114514);
        }
        REQUIRE_EQ(a[11], 10);
    }

    SUBCASE("append")
    {
        TestRingBuffer a;
        a.resize(20, 114514);
        uint32_t buffer[10];
        for (uint32_t i = 0; i < 10; ++i)
        {
            buffer[i] = 114;
        }

        a.append_back({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 26);
        REQUIRE_GE(a.capacity(), 26);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[20], 1);
        REQUIRE_EQ(a[21], 1);
        REQUIRE_EQ(a[22], 4);
        REQUIRE_EQ(a[23], 5);
        REQUIRE_EQ(a[24], 1);
        REQUIRE_EQ(a[25], 4);

        a.append_back(buffer, 5);
        REQUIRE_EQ(a.size(), 31);
        REQUIRE_GE(a.capacity(), 31);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i], 114514);
        }
        REQUIRE_EQ(a[20], 1);
        REQUIRE_EQ(a[21], 1);
        REQUIRE_EQ(a[22], 4);
        REQUIRE_EQ(a[23], 5);
        REQUIRE_EQ(a[24], 1);
        REQUIRE_EQ(a[25], 4);
        for (uint32_t i = 26; i < 31; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }

        a.clear();
        a.resize(20, 114514);

        a.append_front({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 26);
        REQUIRE_GE(a.capacity(), 26);
        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i + 6], 114514);
        }

        a.append_front(buffer, 8);
        REQUIRE_EQ(a.size(), 34);
        REQUIRE_GE(a.capacity(), 34);
        for (uint32_t i = 0; i < 8; ++i)
        {
            REQUIRE_EQ(a[i], 114);
        }
        REQUIRE_EQ(a[8 + 0], 1);
        REQUIRE_EQ(a[8 + 1], 1);
        REQUIRE_EQ(a[8 + 2], 4);
        REQUIRE_EQ(a[8 + 3], 5);
        REQUIRE_EQ(a[8 + 4], 1);
        REQUIRE_EQ(a[8 + 5], 4);
        for (uint32_t i = 0; i < 20; ++i)
        {
            REQUIRE_EQ(a[i + 14], 114514);
        }
    }

    SUBCASE("pop")
    {
        TestRingBuffer a;
        shuffle_ring_buffer(a);
        while (!a.empty())
        {
            a.pop_front();
        }

        a.append_back({ 1, 1, 4, 5, 1, 4 });
        a.pop_front(2);
        REQUIRE_EQ(a.pop_front_get(), 4);
        a.push_front(114514, 3);
        a.pop_back(2);
        REQUIRE_EQ(a.pop_back_get(), 5);
        REQUIRE_EQ(a.pop_back_get(), 114514);
        REQUIRE_EQ(a.pop_back_get(), 114514);
        REQUIRE_EQ(a.pop_back_get(), 114514);

        a.append_front({ 1, 1, 4, 5, 1, 4 });
        a.pop_front_unsafe(2);
        REQUIRE_EQ(a.pop_front_get(), 4);
        a.push_front(114514, 3);
        a.pop_back_unsafe(2);
        REQUIRE_EQ(a.pop_back_get(), 5);
        REQUIRE_EQ(a.pop_back_get(), 114514);
        REQUIRE_EQ(a.pop_back_get(), 114514);
        REQUIRE_EQ(a.pop_back_get(), 114514);
    }

    SUBCASE("modify")
    {
        TestRingBuffer a({ 1, 1, 4, 5, 1, 4 });
        shuffle_ring_buffer(a);
        while (!a.empty())
        {
            a.pop_front();
        }
        a.append_front({ 1, 1, 4, 5, 1, 4 });

        REQUIRE_EQ(a[0], 1);
        REQUIRE_EQ(a[1], 1);
        REQUIRE_EQ(a[2], 4);
        REQUIRE_EQ(a[3], 5);
        REQUIRE_EQ(a[4], 1);
        REQUIRE_EQ(a[5], 4);

        REQUIRE_EQ(a.last(0), 4);
        REQUIRE_EQ(a.last(1), 1);
        REQUIRE_EQ(a.last(2), 5);
        REQUIRE_EQ(a.last(3), 4);
        REQUIRE_EQ(a.last(4), 1);
        REQUIRE_EQ(a.last(5), 1);
    }

    // [needn't test] front & back
}