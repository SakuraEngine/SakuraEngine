#include "SkrBase/containers/bit_set/bit_set.hpp"
#include "SkrTestFramework/framework.hpp"

template <size_t N>
using TestBitset = skr::container::Bitset<N, uint64_t>;

TEST_CASE("test bit set")
{
    const uint32_t u32_max  = std::numeric_limits<uint32_t>::max();
    const uint64_t u64_max  = std::numeric_limits<uint64_t>::max();
    const uint32_t u32_test = 0b01010101010101010101110101010001;
    const uint64_t u64_test = 0b0101010101010101010111010101000101010101010101010111010101000101;
    const uint32_t u32_mask = ((u32_max >> 5) << 5) & ((u32_max << 7) >> 7);
    const uint64_t u64_mask = ((u64_max >> 5) << 5) & ((u64_max << 7) >> 7);

    SUBCASE("test ctor")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> a;
            REQUIRE_EQ(a.data()[0], 0);

            TestBitset<16> b(u32_max);
            REQUIRE_EQ(b.data()[0], u32_max >> 16);

            TestBitset<32> c(u32_max);
            REQUIRE_EQ(c.data()[0], u32_max);

            TestBitset<32> d(u64_max);
            REQUIRE_EQ(d.data()[0], u32_max);
        }

        SUBCASE("block == 1")
        {
            TestBitset<64> a;
            REQUIRE_EQ(a.data()[0], 0);

            TestBitset<64> c(u32_max);
            REQUIRE_EQ(c.data()[0], u32_max);

            TestBitset<64> d(u64_max);
            REQUIRE_EQ(d.data()[0], u64_max);
        }

        SUBCASE("1 < block < 2")
        {
            TestBitset<96> a;
            REQUIRE_EQ(a.data()[0], 0);
            REQUIRE_EQ(a.data()[1], 0);

            TestBitset<96> b(u32_max);
            REQUIRE_EQ(b.data()[0], u32_max);
            REQUIRE_EQ(b.data()[1], 0);

            TestBitset<96> c(u64_max);
            REQUIRE_EQ(c.data()[0], u64_max);
            REQUIRE_EQ(c.data()[1], 0);
        }

        SUBCASE("block == 2")
        {
            TestBitset<128> a;
            REQUIRE_EQ(a.data()[0], 0);
            REQUIRE_EQ(a.data()[1], 0);

            TestBitset<128> b(u32_max);
            REQUIRE_EQ(b.data()[0], u32_max);
            REQUIRE_EQ(b.data()[1], 0);

            TestBitset<128> c(u64_max);
            REQUIRE_EQ(c.data()[0], u64_max);
            REQUIRE_EQ(c.data()[1], 0);
        }

        SUBCASE("block > 2")
        {
            TestBitset<512> a;
            for (int idx = 0; idx < TestBitset<512>::NumBlock; ++idx)
            {
                REQUIRE_EQ(a.data()[idx], 0);
            }

            TestBitset<512> b(u32_max);
            REQUIRE_EQ(b.data()[0], u32_max);
            for (int idx = 1; idx < TestBitset<512>::NumBlock; ++idx)
            {
                REQUIRE_EQ(b.data()[idx], 0);
            }

            TestBitset<512> c(u64_max);
            REQUIRE_EQ(c.data()[0], u64_max);
            for (int idx = 1; idx < TestBitset<512>::NumBlock; ++idx)
            {
                REQUIRE_EQ(c.data()[idx], 0);
            }
        }
    }

    SUBCASE("test copy & move")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> a(u32_test);
            TestBitset<32> b(a);
            TestBitset<32> c(std::move(b));
            TestBitset<32> d(a);

            REQUIRE_EQ(a.data()[0], u32_test);
            REQUIRE_EQ(c.data()[0], u32_test);
            REQUIRE_EQ(d.data()[0], u32_test);
        }

        SUBCASE("block == 1")
        {
            TestBitset<64> a(u64_test);
            TestBitset<64> b(a);
            TestBitset<64> c(std::move(b));
            TestBitset<64> d(a);

            REQUIRE_EQ(a.data()[0], u64_test);
            REQUIRE_EQ(c.data()[0], u64_test);
            REQUIRE_EQ(d.data()[0], u64_test);
        }

        SUBCASE("1 < block < 2")
        {
            TestBitset<96> a(u64_test);
            for (int idx = 0; idx < 96; ++idx)
            {
                a[idx] = idx % 2;
            }
            TestBitset<96> b(a);
            TestBitset<96> c(std::move(b));
            TestBitset<96> d(a);

            for (int idx = 0; idx < 96; ++idx)
            {
                REQUIRE_EQ(a[idx], idx % 2);
                REQUIRE_EQ(c[idx], idx % 2);
                REQUIRE_EQ(d[idx], idx % 2);
            }
        }

        SUBCASE("block > 2")
        {
            TestBitset<512> a;
            for (int idx = 0; idx < 512; ++idx)
            {
                a[idx] = idx % 2;
            }
            TestBitset<512> b(a);
            TestBitset<512> c(std::move(b));
            TestBitset<512> d(a);

            for (int idx = 0; idx < 512; ++idx)
            {
                REQUIRE_EQ(a[idx], idx % 2);
                REQUIRE_EQ(c[idx], idx % 2);
                REQUIRE_EQ(d[idx], idx % 2);
            }
        }
    }

    SUBCASE("assign & move assign")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> a(u32_test);
            TestBitset<32> b;
            TestBitset<32> c;
            TestBitset<32> d;

            b = a;
            c = std::move(b);
            d = a;

            REQUIRE_EQ(a.data()[0], u32_test);
            REQUIRE_EQ(c.data()[0], u32_test);
            REQUIRE_EQ(d.data()[0], u32_test);
        }

        SUBCASE("block == 1")
        {
            TestBitset<64> a(u64_test);
            TestBitset<64> b;
            TestBitset<64> c;
            TestBitset<64> d;

            b = a;
            c = std::move(b);
            d = a;

            REQUIRE_EQ(a.data()[0], u64_test);
            REQUIRE_EQ(c.data()[0], u64_test);
            REQUIRE_EQ(d.data()[0], u64_test);
        }

        SUBCASE("1 < block < 2")
        {
            TestBitset<96> a(u64_test);
            for (int idx = 0; idx < 96; ++idx)
            {
                a[idx] = idx % 2;
            }
            TestBitset<96> b;
            TestBitset<96> c;
            TestBitset<96> d;

            b = a;
            c = std::move(b);
            d = a;

            for (int idx = 0; idx < 96; ++idx)
            {
                REQUIRE_EQ(a[idx], idx % 2);
                REQUIRE_EQ(c[idx], idx % 2);
                REQUIRE_EQ(d[idx], idx % 2);
            }
        }

        SUBCASE("block > 2")
        {
            TestBitset<512> a;
            for (int idx = 0; idx < 512; ++idx)
            {
                a[idx] = idx % 2;
            }
            TestBitset<512> b;
            TestBitset<512> c;
            TestBitset<512> d;

            b = a;
            c = std::move(b);
            d = a;

            for (int idx = 0; idx < 512; ++idx)
            {
                REQUIRE_EQ(a[idx], idx % 2);
                REQUIRE_EQ(c[idx], idx % 2);
                REQUIRE_EQ(d[idx], idx % 2);
            }
        }
    }

    SUBCASE("bit ops")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> mask = u32_mask;

            TestBitset<32> a(u32_test);
            TestBitset<32> b(u32_test);
            TestBitset<32> c(u32_test);

            a &= mask;
            b |= mask;
            c ^= mask;

            REQUIRE_EQ(a.data()[0], u32_test & u32_mask);
            REQUIRE_EQ(b.data()[0], u32_test | u32_mask);
            REQUIRE_EQ(c.data()[0], u32_test ^ u32_mask);

            TestBitset<32> d(u32_test);
            TestBitset<32> e(u32_test);
            TestBitset<32> f(u32_test);

            REQUIRE_EQ((d & mask).data()[0], u32_test & u32_mask);
            REQUIRE_EQ((e | mask).data()[0], u32_test | u32_mask);
            REQUIRE_EQ((f ^ mask).data()[0], u32_test ^ u32_mask);

            TestBitset<32> g(u32_test);

            REQUIRE_EQ((~g).data()[0], ~u32_test);
        }

        SUBCASE("block == 1")
        {
            TestBitset<64> mask = u64_mask;

            TestBitset<64> a(u64_test);
            TestBitset<64> b(u64_test);
            TestBitset<64> c(u64_test);

            a &= mask;
            b |= mask;
            c ^= mask;

            REQUIRE_EQ(a.data()[0], u64_test & u64_mask);
            REQUIRE_EQ(b.data()[0], u64_test | u64_mask);
            REQUIRE_EQ(c.data()[0], u64_test ^ u64_mask);

            TestBitset<64> d(u64_test);
            TestBitset<64> e(u64_test);
            TestBitset<64> f(u64_test);

            REQUIRE_EQ((d & mask).data()[0], u64_test & u64_mask);
            REQUIRE_EQ((e | mask).data()[0], u64_test | u64_mask);
            REQUIRE_EQ((f ^ mask).data()[0], u64_test ^ u64_mask);

            TestBitset<64> g(u64_test);

            REQUIRE_EQ((~g).data()[0], ~u64_test);
        }

        SUBCASE("1 < block < 2")
        {
            TestBitset<96> mask;
            TestBitset<96> value;

            for (int i = 0; i < 96; ++i)
            {
                mask[i]  = i > 16 && i < 68;
                value[i] = i % 96;
            }

            TestBitset<96> a(value);
            TestBitset<96> b(value);
            TestBitset<96> c(value);

            a &= mask;
            b |= mask;
            c ^= mask;

            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(a[i], value[i] && mask[i]);
                REQUIRE_EQ(b[i], value[i] || mask[i]);
                REQUIRE_EQ(c[i], value[i] != mask[i]);
            }

            TestBitset<96> d(value);
            TestBitset<96> e(value);
            TestBitset<96> f(value);

            d = d & mask;
            e = e | mask;
            f = f ^ mask;

            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(d[i], value[i] && mask[i]);
                REQUIRE_EQ(e[i], value[i] || mask[i]);
                REQUIRE_EQ(f[i], value[i] != mask[i]);
            }

            TestBitset<96> g(value);
            g = ~g;

            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(g[i], !value[i]);
            }
        }

        SUBCASE("block == 2")
        {
            TestBitset<128> mask;
            TestBitset<128> value;

            for (int i = 0; i < 128; ++i)
            {
                mask[i]  = i > 37 && i < 96;
                value[i] = i % 128;
            }

            TestBitset<128> a(value);
            TestBitset<128> b(value);
            TestBitset<128> c(value);

            a &= mask;
            b |= mask;
            c ^= mask;

            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(a[i], value[i] && mask[i]);
                REQUIRE_EQ(b[i], value[i] || mask[i]);
                REQUIRE_EQ(c[i], value[i] != mask[i]);
            }

            TestBitset<128> d(value);
            TestBitset<128> e(value);
            TestBitset<128> f(value);

            d = d & mask;
            e = e | mask;
            f = f ^ mask;

            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(d[i], value[i] && mask[i]);
                REQUIRE_EQ(e[i], value[i] || mask[i]);
                REQUIRE_EQ(f[i], value[i] != mask[i]);
            }

            TestBitset<128> g(value);
            g = ~g;

            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(g[i], !value[i]);
            }
        }

        SUBCASE("block > 2")
        {
            TestBitset<512> mask;
            TestBitset<512> value;

            for (int i = 0; i < 512; ++i)
            {
                mask[i]  = i > 37 && i < 96;
                value[i] = i % 512;
            }

            TestBitset<512> a(value);
            TestBitset<512> b(value);
            TestBitset<512> c(value);

            a &= mask;
            b |= mask;
            c ^= mask;

            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(a[i], value[i] && mask[i]);
                REQUIRE_EQ(b[i], value[i] || mask[i]);
                REQUIRE_EQ(c[i], value[i] != mask[i]);
            }

            TestBitset<512> d(value);
            TestBitset<512> e(value);
            TestBitset<512> f(value);

            d = d & mask;
            e = e | mask;
            f = f ^ mask;

            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(d[i], value[i] && mask[i]);
                REQUIRE_EQ(e[i], value[i] || mask[i]);
                REQUIRE_EQ(f[i], value[i] != mask[i]);
            }

            TestBitset<512> g(value);
            g = ~g;

            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(g[i], !value[i]);
            }
        }
    }

    SUBCASE("bit move")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> a(u32_test);
            TestBitset<32> b(u32_test);
            TestBitset<32> c(u32_test);

            a <<= 5;
            b >>= 5;
            c <<= 7;
            c >>= 7;

            REQUIRE_EQ(a.data()[0], u32_test << 5);
            REQUIRE_EQ(b.data()[0], u32_test >> 5);
            REQUIRE_EQ(c.data()[0], (u32_test << 7) >> 7);

            TestBitset<32> d(u32_test);
            TestBitset<32> e(u32_test);
            TestBitset<32> f(u32_test);

            REQUIRE_EQ((d << 5).data()[0], u32_test << 5);
            REQUIRE_EQ((e >> 5).data()[0], u32_test >> 5);
            REQUIRE_EQ(((f << 7) >> 7).data()[0], (u32_test << 7) >> 7);
        }

        SUBCASE("block == 1")
        {
            TestBitset<64> a(u64_test);
            TestBitset<64> b(u64_test);
            TestBitset<64> c(u64_test);

            a <<= 5;
            b >>= 5;
            c <<= 7;
            c >>= 7;

            REQUIRE_EQ(a.data()[0], u64_test << 5);
            REQUIRE_EQ(b.data()[0], u64_test >> 5);
            REQUIRE_EQ(c.data()[0], (u64_test << 7) >> 7);

            TestBitset<64> d(u64_test);
            TestBitset<64> e(u64_test);
            TestBitset<64> f(u64_test);

            REQUIRE_EQ((d << 5).data()[0], u64_test << 5);
            REQUIRE_EQ((e >> 5).data()[0], u64_test >> 5);
            REQUIRE_EQ(((f << 7) >> 7).data()[0], (u64_test << 7) >> 7);
        }

        SUBCASE("1 < block < 2")
        {
            TestBitset<96> value;
            for (int i = 0; i < 96; ++i)
            {
                value[i] = rand() % 2;
            }

            TestBitset<96> a(value);
            TestBitset<96> b(value);
            TestBitset<96> c(value);

            a <<= 5;
            b >>= 5;
            c <<= 7;
            c >>= 7;

            for (int i = 0; i < 96; ++i)
            {
                if (i < 5)
                {
                    REQUIRE_EQ(a[i], false);
                }
                else
                {
                    REQUIRE_EQ(a[i], value[i - 5]);
                }

                if (i > 90)
                {
                    REQUIRE_EQ(b[i], false);
                }
                else
                {
                    REQUIRE_EQ(b[i], value[i + 5]);
                }

                if (i > 89)
                {
                    REQUIRE_EQ(c[i], false);
                }
                else
                {
                    REQUIRE_EQ(c[i], value[i]);
                }
            }
        }

        SUBCASE("block == 2")
        {
            TestBitset<128> value;
            for (int i = 0; i < 128; ++i)
            {
                value[i] = rand() % 2;
            }

            TestBitset<128> a(value);
            TestBitset<128> b(value);
            TestBitset<128> c(value);

            a <<= 5;
            b >>= 5;
            c <<= 7;
            c >>= 7;

            for (int i = 0; i < 128; ++i)
            {
                if (i < 5)
                {
                    REQUIRE_EQ(a[i], false);
                }
                else
                {
                    REQUIRE_EQ(a[i], value[i - 5]);
                }

                if (i > 122)
                {
                    REQUIRE_EQ(b[i], false);
                }
                else
                {
                    REQUIRE_EQ(b[i], value[i + 5]);
                }

                if (i > 120)
                {
                    REQUIRE_EQ(c[i], false);
                }
                else
                {
                    REQUIRE_EQ(c[i], value[i]);
                }
            }
        }

        SUBCASE("block > 2")
        {
            TestBitset<512> value;
            for (int i = 0; i < 512; ++i)
            {
                value[i] = rand() % 2;
            }

            TestBitset<512> a(value);
            TestBitset<512> b(value);
            TestBitset<512> c(value);

            a <<= 5;
            b >>= 5;
            c <<= 7;
            c >>= 7;

            for (int i = 0; i < 512; ++i)
            {
                if (i < 5)
                {
                    REQUIRE_EQ(a[i], false);
                }
                else
                {
                    REQUIRE_EQ(a[i], value[i - 5]);
                }

                if (i > 506)
                {
                    REQUIRE_EQ(b[i], false);
                }
                else
                {
                    REQUIRE_EQ(b[i], value[i + 5]);
                }

                if (i > 504)
                {
                    REQUIRE_EQ(c[i], false);
                }
                else
                {
                    REQUIRE_EQ(c[i], value[i]);
                }
            }
        }
    }

    SUBCASE("bit modify")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> a(u32_test);
            a.set();
            REQUIRE_EQ(a.data()[0], u32_max);

            a.reset();
            REQUIRE_EQ(a.data()[0], 0);

            a.flip();
            REQUIRE_EQ(a.data()[0], u32_max);

            a.flip();
            REQUIRE_EQ(a.data()[0], 0);

            a.set(5, true);
            REQUIRE_EQ(a.data()[0], 1 << 5);

            a.set(5, false);
            REQUIRE_EQ(a.data()[0], 0);

            a.set();
            a.reset(5);
            REQUIRE_EQ(a.data()[0], ~static_cast<uint32_t>(1 << 5));

            a.flip(5);
            REQUIRE_EQ(a.data()[0], u32_max);
        }
        SUBCASE("block == 1")
        {
            TestBitset<64> a(u64_test);
            a.set();
            REQUIRE_EQ(a.data()[0], u64_max);

            a.reset();
            REQUIRE_EQ(a.data()[0], 0);

            a.flip();
            REQUIRE_EQ(a.data()[0], u64_max);

            a.flip();
            REQUIRE_EQ(a.data()[0], 0);

            a.set(5, true);
            REQUIRE_EQ(a.data()[0], 1 << 5);

            a.set(5, false);
            REQUIRE_EQ(a.data()[0], 0);

            a.set();
            a.reset(5);
            REQUIRE_EQ(a.data()[0], ~static_cast<uint64_t>(1 << 5));

            a.flip(5);
            REQUIRE_EQ(a.data()[0], u64_max);
        }
        SUBCASE("1 < block < 2")
        {
            TestBitset<96> value;
            for (int i = 0; i < 96; ++i)
            {
                value[i] = rand() % 2;
            }

            TestBitset<96> a(value);
            a.set();
            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(a[i], true);
            }

            a.reset();
            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(a[i], false);
            }

            a = value;
            a.flip();
            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_NE(a[i], value[i]);
            }

            a.reset();
            a.set(83, true);
            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(a[i], i == 83);
            }

            a.set();
            a.reset(83);
            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(a[i], i != 83);
            }

            a.flip(83);
            for (int i = 0; i < 96; ++i)
            {
                REQUIRE_EQ(a[i], true);
            }
        }
        SUBCASE("block == 2")
        {
            TestBitset<128> value;
            for (int i = 0; i < 128; ++i)
            {
                value[i] = rand() % 2;
            }

            TestBitset<128> a(value);
            a.set();
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(a[i], true);
            }

            a.reset();
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(a[i], false);
            }

            a = value;
            a.flip();
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_NE(a[i], value[i]);
            }

            a.reset();
            a.set(83, true);
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(a[i], i == 83);
            }

            a.set();
            a.reset(83);
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(a[i], i != 83);
            }

            a.flip(83);
            for (int i = 0; i < 128; ++i)
            {
                REQUIRE_EQ(a[i], true);
            }
        }
        SUBCASE("block > 2")
        {
            TestBitset<512> value;
            for (int i = 0; i < 512; ++i)
            {
                value[i] = rand() % 2;
            }

            TestBitset<512> a(value);
            a.set();
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(a[i], true);
            }

            a.reset();
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(a[i], false);
            }

            a = value;
            a.flip();
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_NE(a[i], value[i]);
            }

            a.reset();
            a.set(83, true);
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(a[i], i == 83);
            }

            a.set();
            a.reset(83);
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(a[i], i != 83);
            }

            a.flip(83);
            for (int i = 0; i < 512; ++i)
            {
                REQUIRE_EQ(a[i], true);
            }
        }
    }

    // [needn't test] visitor

    SUBCASE("compare")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> a(u64_test);
            TestBitset<32> b(u64_test);
            TestBitset<32> c(u64_test);
            TestBitset<32> d(u64_max);
            c.reset();

            REQUIRE_EQ(a, b);
            REQUIRE_NE(a, d);
            REQUIRE_NE(a, c);
            d.reset();
            REQUIRE_EQ(c, d);
        }
        SUBCASE("block == 1")
        {
            TestBitset<64> a(u64_test);
            TestBitset<64> b(u64_test);
            TestBitset<64> c(u64_test);
            TestBitset<64> d(u64_max);
            c.reset();

            REQUIRE_EQ(a, b);
            REQUIRE_NE(a, d);
            REQUIRE_NE(a, c);
            d.reset();
            REQUIRE_EQ(c, d);
        }
        SUBCASE("1 < block < 2")
        {
            TestBitset<96> value, full;
            for (int i = 0; i < 96; ++i)
            {
                value[i] = rand() % 2;
                full[i]  = true;
            }

            TestBitset<96> a(value);
            TestBitset<96> b(value);
            TestBitset<96> c(value);
            TestBitset<96> d(full);
            c.reset();

            REQUIRE_EQ(a, b);
            REQUIRE_NE(a, d);
            REQUIRE_NE(a, c);
            d.reset();
            REQUIRE_EQ(c, d);
        }
        SUBCASE("block == 2")
        {
            TestBitset<128> value, full;
            for (int i = 0; i < 128; ++i)
            {
                value[i] = rand() % 2;
                full[i]  = true;
            }

            TestBitset<128> a(value);
            TestBitset<128> b(value);
            TestBitset<128> c(value);
            TestBitset<128> d(full);
            c.reset();

            REQUIRE_EQ(a, b);
            REQUIRE_NE(a, d);
            REQUIRE_NE(a, c);
            d.reset();
            REQUIRE_EQ(c, d);
        }
        SUBCASE("block > 2")
        {
            TestBitset<512> value, full;
            for (int i = 0; i < 512; ++i)
            {
                value[i] = rand() % 2;
                full[i]  = true;
            }

            TestBitset<512> a(value);
            TestBitset<512> b(value);
            TestBitset<512> c(value);
            TestBitset<512> d(full);
            c.reset();

            REQUIRE_EQ(a, b);
            REQUIRE_NE(a, d);
            REQUIRE_NE(a, c);
            d.reset();
            REQUIRE_EQ(c, d);
        }
    }

    SUBCASE("summary")
    {
        SUBCASE("block < 1")
        {
            TestBitset<32> value, full, empty;
            int32_t        count = 0;
            for (int i = 0; i < 32; ++i)
            {
                bool v = rand() % 2;
                if (v)
                {
                    ++count;
                }
                value[i] = v;
                full[i]  = true;
                empty[i] = false;
            }

            REQUIRE(full.all());
            REQUIRE(full.any());
            REQUIRE_FALSE(empty.any());
            REQUIRE_EQ(value.any(), count > 0);
            REQUIRE(empty.none());
            REQUIRE_EQ(value.count(), count);
        }
        SUBCASE("block == 1")
        {
            TestBitset<64> value, full, empty;
            int32_t        count = 0;
            for (int i = 0; i < 64; ++i)
            {
                bool v = rand() % 2;
                if (v)
                {
                    ++count;
                }
                value[i] = v;
                full[i]  = true;
                empty[i] = false;
            }

            REQUIRE(full.all());
            REQUIRE(full.any());
            REQUIRE_FALSE(empty.any());
            REQUIRE_EQ(value.any(), count > 0);
            REQUIRE(empty.none());
            REQUIRE_EQ(value.count(), count);
        }
        SUBCASE("1 < block < 2")
        {
            TestBitset<96> value, full, empty;
            int32_t        count = 0;
            for (int i = 0; i < 96; ++i)
            {
                bool v = rand() % 2;
                if (v)
                {
                    ++count;
                }
                value[i] = v;
                full[i]  = true;
                empty[i] = false;
            }

            REQUIRE(full.all());
            REQUIRE(full.any());
            REQUIRE_FALSE(empty.any());
            REQUIRE_EQ(value.any(), count > 0);
            REQUIRE(empty.none());
            REQUIRE_EQ(value.count(), count);
        }
        SUBCASE("block == 2")
        {
            TestBitset<128> value, full, empty;
            int32_t         count = 0;
            for (int i = 0; i < 128; ++i)
            {
                bool v = rand() % 2;
                if (v)
                {
                    ++count;
                }
                value[i] = v;
                full[i]  = true;
                empty[i] = false;
            }

            REQUIRE(full.all());
            REQUIRE(full.any());
            REQUIRE_FALSE(empty.any());
            REQUIRE_EQ(value.any(), count > 0);
            REQUIRE(empty.none());
            REQUIRE_EQ(value.count(), count);
        }
        SUBCASE("block > 2")
        {
            TestBitset<512> value, full, empty;
            int32_t         count = 0;
            for (int i = 0; i < 512; ++i)
            {
                bool v = rand() % 2;
                if (v)
                {
                    ++count;
                }
                value[i] = v;
                full[i]  = true;
                empty[i] = false;
            }

            REQUIRE(full.all());
            REQUIRE(full.any());
            REQUIRE_FALSE(empty.any());
            REQUIRE_EQ(value.any(), count > 0);
            REQUIRE(empty.none());
            REQUIRE_EQ(value.count(), count);
        }
    }
}