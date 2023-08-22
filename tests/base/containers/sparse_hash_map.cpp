#include "SkrTestFramework/framework.hpp"
#include "skr_test_allocator.hpp"

#include "SkrBase/tools/hash.hpp"
#include "SkrBase/containers/sparse_hash_map/sparse_hash_map.hpp"
#include <chrono>

TEST_CASE("test sparse hash map")
{
    using namespace skr;
    using KeyType   = int32_t;
    using ValueType = int32_t;
    using PairType  = KVPair<KeyType, ValueType>;

    using TestHashMap = SparseHashMap<KeyType, ValueType, uint64_t, size_t, Hash<KeyType>, Equal<KeyType>, false, SkrTestAllocator>;

    SUBCASE("ctor & dtor")
    {
        TestHashMap a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.bucket_size(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);

        TestHashMap b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 100);
        REQUIRE_EQ(b.bucket_size(), 0);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_FALSE(b.has_data(i));
        }

        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE_GE(c.bucket_size(), 4);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));

        PairType    data[] = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
        TestHashMap d(data, 6);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE_GE(c.bucket_size(), 4);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));
    }

    SUBCASE("copy & move")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.sparse_size(), 3);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 3);
        REQUIRE_GE(a.bucket_size(), 4);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));

        TestHashMap b(a);
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 3);
        REQUIRE_GE(a.bucket_size(), 4);
        REQUIRE(b.contain(1));
        REQUIRE(b.contain(4));
        REQUIRE(b.contain(5));

        auto        old_capacity = a.capacity();
        TestHashMap c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.bucket_size(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));
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
        REQUIRE_GE(b.bucket_size(), 4);
        REQUIRE(b.contain(1));
        REQUIRE(b.contain(4));
        REQUIRE(b.contain(5));
        REQUIRE_FALSE(b.contain(114514));

        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.bucket_size(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE_GE(c.bucket_size(), 3);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));
        REQUIRE_FALSE(c.contain(114514));
        REQUIRE_FALSE(a.contain(1));
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
        REQUIRE_GE(a.bucket_size(), 8);

        auto old_capacity    = a.capacity();
        auto old_bucket_size = a.bucket_size();
        a.clear();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), old_capacity);
        REQUIRE_EQ(a.bucket_size(), old_bucket_size);

        a.release();
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.bucket_size(), 0);

        a.release(5);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 5);
        REQUIRE_GE(a.bucket_size(), 0);

        a.reserve(100);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 100);
        REQUIRE_GE(a.bucket_size(), 0);

        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 100);
        REQUIRE_GE(a.bucket_size(), 8);

        a.remove(11451);
        a.remove(114514);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_GE(a.bucket_size(), 4);

        a.clear();
        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(11451);
        a.remove(1);
        a.compact();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contain(11));
        REQUIRE(a.contain(114));
        REQUIRE(a.contain(1145));
        REQUIRE(a.contain(114514));

        a.clear();
        a.append({ { 1, 1 }, { 11, 11 }, { 114, 114 }, { 1145, 1145 }, { 11451, 11451 }, { 114514, 114514 } });
        a.remove(114);
        a.remove(11);
        a.compact_stable();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(1145));
        REQUIRE(a.contain(11451));
        REQUIRE(a.contain(114514));
    }

    // [needn't test] data op
    // [needn't test] bucket op

    SUBCASE("add")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.add(1, 1);
        a.add(4, 4);
        a.add(10, 10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE_GE(a.bucket_size(), 4);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));

        a.add_ex(
        Hash<KeyType>()(100),
        [](const KeyType& v) { return v == 100; },
        [](void* p) { new (p) PairType(100, 100); });
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 5);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 5);
        REQUIRE_GE(a.bucket_size(), 8);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));
        REQUIRE(a.contain(100));

        auto ref = a.add_ex_unsafe(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        new (ref.data) PairType(114514, 114514);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE_GE(a.bucket_size(), 8);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));
        REQUIRE(a.contain(100));
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
        REQUIRE_GE(a.bucket_size(), 4);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));
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
        REQUIRE_GE(a.bucket_size(), 8);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(114));
        REQUIRE(a.contain(514));
        REQUIRE(a.contain(114514));

        TestHashMap b;
        b.append(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE_GE(b.bucket_size(), 8);
        REQUIRE(b.contain(1));
        REQUIRE(b.contain(4));
        REQUIRE(b.contain(5));
        REQUIRE(b.contain(114));
        REQUIRE(b.contain(514));
        REQUIRE(b.contain(114514));
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
        REQUIRE_GE(a.bucket_size(), 4);
        REQUIRE(a.contain(5));
        REQUIRE_FALSE(a.contain(1));
        REQUIRE_FALSE(a.contain(4));

        a.append({ { 114514, 114514 }, { 114514, 114514 }, { 114514, 114514 }, { 114, 114 }, { 514, 514 } });
        a.remove_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(114));
        REQUIRE(a.contain(514));
        REQUIRE_FALSE(a.contain(114514));
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

    SUBCASE("contain")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE_FALSE(a.contain(114514));
        REQUIRE(a.contain_ex(Hash<KeyType>()(1), [](const KeyType& key) { return key == 1; }));
        REQUIRE(a.contain_ex(Hash<KeyType>()(4), [](const KeyType& key) { return key == 4; }));
        REQUIRE(a.contain_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; }));
        REQUIRE_FALSE(a.contain_ex(Hash<KeyType>()(114514), [](const KeyType& key) { return key == 114514; }));
    }

    SUBCASE("sort")
    {
        srand(std::chrono::system_clock::now().time_since_epoch().count());
        TestHashMap a(100);
        for (auto i = 0; i < 100; ++i)
        {
            auto k = rand() % 100;
            while (a.contain(k))
            {
                k = rand() % 100;
            }
            a.add(k, k * 5);
        }
        a.sort();
        REQUIRE_EQ(a.size(), 100);
        REQUIRE_EQ(a.sparse_size(), 100);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 100);
        REQUIRE_GE(a.bucket_size(), 64);
        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(a.contain(i));
            REQUIRE(a.find(i)->value == i * 5);
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data.key, i);
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data.value, i * 5);
        }
    }

    // [needn't test] set ops
}
