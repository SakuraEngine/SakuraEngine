#include "SkrTestFramework/framework.hpp"
#include "container_test_types.hpp"
#include <chrono>

TEST_CASE("test sparse hash map")
{
    using namespace skr;
    using KeyType   = int32_t;
    using ValueType = int32_t;
    using PairType  = container::KVPair<KeyType, ValueType>;

    using TestHashMap = SparseHashMap<KeyType, ValueType>;

    SUBCASE("ctor & dtor")
    {
        TestHashMap a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);

        TestHashMap b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 100);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_FALSE(b.has_data(i));
        }

        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));

        PairType    data[] = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
        TestHashMap d(data, 6);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
    }

    SUBCASE("copy & move")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.sparse_size(), 3);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 3);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));

        TestHashMap b(a);
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 3);
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));

        auto        old_capacity = a.capacity();
        TestHashMap c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
    }

    SUBCASE("assign & move assign")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        TestHashMap b({ { 114514, 114514 }, { 114514, 114514 }, { 1, 1 }, { 1, 1 }, { 4, 4 } });
        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 514, 514 }, { 514, 514 }, { 514, 514 } });

        b = a;
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 3);
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));
        REQUIRE_FALSE(b.contains(114514));

        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
        REQUIRE_FALSE(c.contains(114514));
        REQUIRE_FALSE(a.contains(1));
    }

    // [needn't test] getter
    // [needn't test] validate

    SUBCASE("memory op")
    {
        TestHashMap a({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(11);
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 1);
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

        a.release(5);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 5);

        a.reserve(100);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 100);

        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 100);

        a.remove(11451);
        a.remove(114514);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);

        a.clear();
        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(11451);
        a.remove(1);
        a.compact();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contains(11));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(1145));
        REQUIRE(a.contains(114514));

        a.clear();
        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(114);
        a.remove(11);
        a.compact_stable();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(1145));
        REQUIRE(a.contains(11451));
        REQUIRE(a.contains(114514));
    }

    // [needn't test] data op
    // [needn't test] bucket op

    SUBCASE("find_or_add")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.find_or_add(1, 1);
        a.find_or_add(4, 4);
        a.find_or_add(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));

        a.find_or_add_ex(
        Hash<KeyType>()(100),
        [](const KeyType& v) { return v == 100; },
        [](void* p) { new (p) PairType(100, 100); });
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 5);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 5);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE(a.contains(100));

        auto ref = a.find_or_add_ex_unsafe(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        new (ref.data) PairType(114514, 114514);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE(a.contains(100));
    }

    SUBCASE("find_or_add or assign")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.add_or_assign(1, 2);
        a.add_or_assign(4, 5);
        a.add_or_assign(5, 6);
        a.add_or_assign(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE_EQ(a.find(1)->value, 2);
        REQUIRE_EQ(a.find(4)->value, 5);
        REQUIRE_EQ(a.find(5)->value, 6);
        REQUIRE_EQ(a.find(10)->value, 10);
    }

    SUBCASE("emplace")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } } });
        a.emplace(1, 1);
        a.emplace(1, 1);
        a.emplace(4, 4);
        a.emplace(5, 5);
        a.emplace(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
    }

    SUBCASE("append")
    {
        TestHashMap a;
        a.append({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.append({ { 114514, 114514 }, { 114514, 114514 }, { 114514, 114514 }, { 114, 114 }, { 514, 514 } });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE(a.contains(114514));

        TestHashMap b;
        b.append(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));
        REQUIRE(b.contains(114));
        REQUIRE(b.contains(514));
        REQUIRE(b.contains(114514));
    }

    SUBCASE("remove")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } } });
        a.remove(1);
        a.remove(4);
        REQUIRE_EQ(a.size(), 1);
        REQUIRE_LE(a.sparse_size(), 3);
        REQUIRE_LE(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 3);
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(1));
        REQUIRE_FALSE(a.contains(4));

        a.append({ { 114514, 114514 }, { 114514, 114514 }, { 114514, 114514 }, { 114, 114 }, { 514, 514 } });
        a.remove_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE_FALSE(a.contains(114514));
    }

    SUBCASE("erase")
    {
        TestHashMap a(100), b(100);
        for (int32_t i = 0; i < 100; ++i)
        {
            a.find_or_add(i, i + 1);
            b.find_or_add(i, i + 1);
        }

        for (auto it = a.begin(); it != a.end();)
        {
            if (it->key % 3 == 0)
            {
                it = a.erase(it);
            }
            else
            {
                ++it;
            }
        }

        const TestHashMap& cb = b;
        for (auto it = cb.begin(); it != cb.end();)
        {
            if (it->key % 3 == 0)
            {
                it = b.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (int32_t i = 0; i < 100; ++i)
        {
            if (i % 3 == 0)
            {
                REQUIRE_FALSE(a.contains(i));
                REQUIRE_FALSE(b.contains(i));
            }
            else
            {
                REQUIRE(a.contains(i));
                REQUIRE(b.contains(i));
            }
        }
    }

    SUBCASE("find")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        {
            auto ref = a.find(1);
            REQUIRE(ref);
            REQUIRE_EQ(ref->key, 1);
            REQUIRE_EQ(ref->value, 1);
        }
        {
            auto ref = a.find_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref->key, 5);
            REQUIRE_EQ(ref->value, 114514);
        }
    }

    SUBCASE("contains")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(114514));
        REQUIRE(a.contains_ex(Hash<KeyType>()(1), [](const KeyType& key) { return key == 1; }));
        REQUIRE(a.contains_ex(Hash<KeyType>()(4), [](const KeyType& key) { return key == 4; }));
        REQUIRE(a.contains_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; }));
        REQUIRE_FALSE(a.contains_ex(Hash<KeyType>()(114514), [](const KeyType& key) { return key == 114514; }));
    }

    SUBCASE("sort")
    {
        srand(std::chrono::system_clock::now().time_since_epoch().count());
        TestHashMap a(100);
        for (auto i = 0; i < 100; ++i)
        {
            auto k = rand() % 100;
            while (a.contains(k))
            {
                k = rand() % 100;
            }
            a.find_or_add(k, k * 5);
        }
        a.sort();
        REQUIRE_EQ(a.size(), 100);
        REQUIRE_EQ(a.sparse_size(), 100);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 100);
        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(a.contains(i));
            REQUIRE(a.find(i)->value == i * 5);
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data.key, i);
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data.value, i * 5);
        }
    }

    // [needn't test] set ops

    // test iterator
    SUBCASE("iterator")
    {
        TestHashMap a;
        for (auto [k, v] : a)
        {
            printf("%d: T%d\n", k, v);
        }
    }
}

TEST_CASE("test fixed sparse hash map")
{
    using namespace skr;
    using KeyType   = int32_t;
    using ValueType = int32_t;
    using PairType  = container::KVPair<KeyType, ValueType>;

    static constexpr size_t kFixedCapacity = 200;
    using TestHashMap                      = FixedSparseHashMap<KeyType, ValueType, kFixedCapacity>;

    SUBCASE("ctor & dtor")
    {
        TestHashMap a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);
        REQUIRE_NE(a.data_arr().data(), nullptr);

        TestHashMap b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), kFixedCapacity);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_FALSE(b.has_data(i));
        }

        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), kFixedCapacity);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));

        PairType    data[] = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
        TestHashMap d(data, 6);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), kFixedCapacity);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
    }

    SUBCASE("copy & move")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.sparse_size(), 3);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));

        TestHashMap b(a);
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), kFixedCapacity);
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));

        TestHashMap c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);
        REQUIRE_NE(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), kFixedCapacity);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
    }

    SUBCASE("assign & move assign")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        TestHashMap b({ { 114514, 114514 }, { 114514, 114514 }, { 1, 1 }, { 1, 1 }, { 4, 4 } });
        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 514, 514 }, { 514, 514 }, { 514, 514 } });

        b = a;
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), kFixedCapacity);
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));
        REQUIRE_FALSE(b.contains(114514));

        c = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);
        REQUIRE_NE(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), kFixedCapacity);
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
        REQUIRE_FALSE(c.contains(114514));
        REQUIRE_FALSE(a.contains(1));
    }

    // [needn't test] getter
    // [needn't test] validate

    SUBCASE("memory op")
    {
        TestHashMap a({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(11);
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 1);
        REQUIRE_GE(a.capacity(), kFixedCapacity);

        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);

        a.release(5);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);

        a.reserve(100);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);

        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);

        a.remove(11451);
        a.remove(114514);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);

        a.clear();
        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(11451);
        a.remove(1);
        a.compact();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(11));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(1145));
        REQUIRE(a.contains(114514));

        a.clear();
        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(114);
        a.remove(11);
        a.compact_stable();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(1145));
        REQUIRE(a.contains(11451));
        REQUIRE(a.contains(114514));
    }

    // [needn't test] data op
    // [needn't test] bucket op

    SUBCASE("find_or_add")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.find_or_add(1, 1);
        a.find_or_add(4, 4);
        a.find_or_add(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));

        a.find_or_add_ex(
        Hash<KeyType>()(100),
        [](const KeyType& v) { return v == 100; },
        [](void* p) { new (p) PairType(100, 100); });
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 5);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE(a.contains(100));

        auto ref = a.find_or_add_ex_unsafe(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        new (ref.data) PairType(114514, 114514);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE(a.contains(100));
    }

    SUBCASE("find_or_add or assign")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.add_or_assign(1, 2);
        a.add_or_assign(4, 5);
        a.add_or_assign(5, 6);
        a.add_or_assign(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE_EQ(a.find(1)->value, 2);
        REQUIRE_EQ(a.find(4)->value, 5);
        REQUIRE_EQ(a.find(5)->value, 6);
        REQUIRE_EQ(a.find(10)->value, 10);
    }

    SUBCASE("emplace")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } } });
        a.emplace(1, 1);
        a.emplace(1, 1);
        a.emplace(4, 4);
        a.emplace(5, 5);
        a.emplace(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
    }

    SUBCASE("append")
    {
        TestHashMap a;
        a.append({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.append({ { 114514, 114514 }, { 114514, 114514 }, { 114514, 114514 }, { 114, 114 }, { 514, 514 } });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE(a.contains(114514));

        TestHashMap b;
        b.append(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), kFixedCapacity);
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));
        REQUIRE(b.contains(114));
        REQUIRE(b.contains(514));
        REQUIRE(b.contains(114514));
    }

    SUBCASE("remove")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } } });
        a.remove(1);
        a.remove(4);
        REQUIRE_EQ(a.size(), 1);
        REQUIRE_LE(a.sparse_size(), 3);
        REQUIRE_LE(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), kFixedCapacity);
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(1));
        REQUIRE_FALSE(a.contains(4));

        a.append({ { 114514, 114514 }, { 114514, 114514 }, { 114514, 114514 }, { 114, 114 }, { 514, 514 } });
        a.remove_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE_FALSE(a.contains(114514));
    }

    SUBCASE("erase")
    {
        TestHashMap a(kFixedCapacity), b(kFixedCapacity);
        for (int32_t i = 0; i < kFixedCapacity; ++i)
        {
            a.find_or_add(i, i + 1);
            b.find_or_add(i, i + 1);
        }

        for (auto it = a.begin(); it != a.end();)
        {
            if (it->key % 3 == 0)
            {
                it = a.erase(it);
            }
            else
            {
                ++it;
            }
        }

        const TestHashMap& cb = b;
        for (auto it = cb.begin(); it != cb.end();)
        {
            if (it->key % 3 == 0)
            {
                it = b.erase(it);
            }
            else
            {
                ++it;
            }
        }

        for (int32_t i = 0; i < kFixedCapacity; ++i)
        {
            if (i % 3 == 0)
            {
                REQUIRE_FALSE(a.contains(i));
                REQUIRE_FALSE(b.contains(i));
            }
            else
            {
                REQUIRE(a.contains(i));
                REQUIRE(b.contains(i));
            }
        }
    }

    SUBCASE("find")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        {
            auto ref = a.find(1);
            REQUIRE(ref);
            REQUIRE_EQ(ref->key, 1);
            REQUIRE_EQ(ref->value, 1);
        }
        {
            auto ref = a.find_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref->key, 5);
            REQUIRE_EQ(ref->value, 114514);
        }
    }

    SUBCASE("contains")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(114514));
        REQUIRE(a.contains_ex(Hash<KeyType>()(1), [](const KeyType& key) { return key == 1; }));
        REQUIRE(a.contains_ex(Hash<KeyType>()(4), [](const KeyType& key) { return key == 4; }));
        REQUIRE(a.contains_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; }));
        REQUIRE_FALSE(a.contains_ex(Hash<KeyType>()(114514), [](const KeyType& key) { return key == 114514; }));
    }

    SUBCASE("sort")
    {
        srand(std::chrono::system_clock::now().time_since_epoch().count());
        TestHashMap a(100);
        for (auto i = 0; i < 100; ++i)
        {
            auto k = rand() % 100;
            while (a.contains(k))
            {
                k = rand() % 100;
            }
            a.find_or_add(k, k * 5);
        }
        a.sort();
        REQUIRE_EQ(a.size(), 100);
        REQUIRE_EQ(a.sparse_size(), 100);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), kFixedCapacity);
        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(a.contains(i));
            REQUIRE(a.find(i)->value == i * 5);
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data.key, i);
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data.value, i * 5);
        }
    }

    // [needn't test] set ops

    // [needn't test] test iterator
}
