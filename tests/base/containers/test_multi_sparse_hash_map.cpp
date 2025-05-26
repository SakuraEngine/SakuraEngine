#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"
#include <chrono>

template <typename KeyType, typename PairType, typename TestHashMap, typename ModifyCapacity, typename ClampCapacity>
void template_test_sparse_hash_map(ModifyCapacity&& capacity_of, ClampCapacity&& clamp_capacity)
{
    using skr::Hash;
    using namespace skr::test_container;

    SUBCASE("ctor & dtor")
    {
        TestHashMap a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));

        TestHashMap b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(100));

        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), capacity_of(6));
        REQUIRE_EQ(c.count(1), 3);
        REQUIRE_EQ(c.count(4), 2);
        REQUIRE_EQ(c.count(5), 1);

        PairType    data[] = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
        TestHashMap d(data, 6);
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), capacity_of(6));
        REQUIRE_EQ(c.count(1), 3);
        REQUIRE_EQ(c.count(4), 2);
        REQUIRE_EQ(c.count(5), 1);
    }

    SUBCASE("copy & move")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.reserve(capacity_of(114514)); // test for bad capacity copy
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);

        TestHashMap b(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(6));
        REQUIRE_EQ(b.count(1), 3);
        REQUIRE_EQ(b.count(4), 2);
        REQUIRE_EQ(b.count(5), 1);

        auto        old_capacity = a.capacity();
        TestHashMap c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        REQUIRE_EQ(c.count(1), 3);
        REQUIRE_EQ(c.count(4), 2);
        REQUIRE_EQ(c.count(5), 1);
    }

    SUBCASE("assign & move assign")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        TestHashMap b({ { 114514, 114514 }, { 114514, 114514 }, { 1, 1 }, { 1, 1 }, { 4, 4 } });
        TestHashMap c({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 514, 514 }, { 514, 514 }, { 514, 514 } });

        b = a;
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(6));
        REQUIRE_EQ(b.count(1), 3);
        REQUIRE_EQ(b.count(4), 2);
        REQUIRE_EQ(b.count(5), 1);
        REQUIRE_FALSE(b.contains(114514));

        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        REQUIRE_EQ(c.count(1), 3);
        REQUIRE_EQ(c.count(4), 2);
        REQUIRE_EQ(c.count(5), 1);
        REQUIRE_FALSE(c.contains(114514));
        REQUIRE_FALSE(a.contains(1));
    }

    // [needn't test] getter
    // [needn't test] validate

    // [test in single map] memory op

    // [needn't test] rehash

    SUBCASE("add")
    {
        TestHashMap a;
        a.add(1, 1);
        a.add(1, 1);
        a.add(4, 4);
        a.add(5, 5);
        a.add(1, 1);
        a.add(4, 4);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);

        a.add_ex(
            Hash<KeyType>()(100),
            [](auto* p) { *p = PairType{ 100, 100 }; }
        );
        REQUIRE_EQ(a.size(), 7);
        REQUIRE_EQ(a.count(100), 1);

        {
            auto ref = a.add_ex_unsafe(Hash<KeyType>()(114514));
            new (ref.ptr()) PairType(114514, 514);
            REQUIRE_EQ(a.size(), 8);
            REQUIRE_EQ(a.count(114514), 1);
            REQUIRE_EQ(a.find(114514).value(), 514);
        }

        {
            auto ref    = a.add_unsafe(114);
            ref.value() = 514;
            REQUIRE_EQ(a.size(), 9);
            REQUIRE_EQ(a.count(114), 1);
            REQUIRE_EQ(a.find(114).value(), 514);
        }

        {
            a.add_default(1140);
            REQUIRE_EQ(a.size(), 10);
            REQUIRE_EQ(a.count(1140), 1);
            REQUIRE_EQ(a.find(1140).value(), 0);
        }

        {
            a.add_zeroed(1141);
            REQUIRE_EQ(a.size(), 11);
            REQUIRE_EQ(a.count(1141), 1);
            REQUIRE_EQ(a.find(1141).value(), 0);
        }
    }

    SUBCASE("emplace")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.emplace(1145, 114514);
        REQUIRE_EQ(a.size(), 7);
        REQUIRE_EQ(a.count(1145), 1);
        REQUIRE_EQ(a.find(1145).value(), 114514);
    }

    SUBCASE("append")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        a.append({ { 1, 2 }, { 1, 2 }, { 4, 5 }, { 5, 6 }, { 1, 2 }, { 4, 5 } });
        REQUIRE_EQ(a.size(), 12);
        REQUIRE_EQ(a.count(1), 6);
        REQUIRE_EQ(a.count(4), 4);
        REQUIRE_EQ(a.count(5), 2);

        TestHashMap b({ { 114, 114 }, { 514, 514 }, { 114514, 114514 } });
        a.append(b);
        REQUIRE_EQ(a.size(), 15);
        REQUIRE_EQ(a.count(114), 1);
        REQUIRE_EQ(a.count(514), 1);
        REQUIRE_EQ(a.count(114514), 1);

        PairType c[] = { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } };
        a.append(c, 6);
        REQUIRE_EQ(a.size(), 21);
        REQUIRE_EQ(a.count(1), 9);
        REQUIRE_EQ(a.count(4), 6);
        REQUIRE_EQ(a.count(5), 3);
    }

    SUBCASE("remove")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } } });
        a.remove(1);
        a.remove(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.count(1), 2);
        REQUIRE_EQ(a.count(4), 1);
        REQUIRE_EQ(a.count(5), 1);

        a.append({ { 114514, 114514 }, { 114514, 114514 }, { 114514, 114514 }, { 114, 114 }, { 514, 514 } });
        a.remove_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; });
        REQUIRE_EQ(a.size(), 8);
        REQUIRE_EQ(a.count(114514), 2);
        REQUIRE_EQ(a.count(114), 1);
        REQUIRE_EQ(a.count(514), 1);
        REQUIRE_EQ(a.count(1), 2);
        REQUIRE_EQ(a.count(4), 1);
        REQUIRE_EQ(a.count(5), 1);

        REQUIRE_EQ(a.remove_all(1), 2);
        REQUIRE_EQ(a.remove_all(4), 1);
        REQUIRE_EQ(a.remove_all_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }), 2);
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.count(114514), 0);
        REQUIRE_EQ(a.count(114), 1);
        REQUIRE_EQ(a.count(514), 1);
        REQUIRE_EQ(a.count(1), 0);
        REQUIRE_EQ(a.count(4), 0);
        REQUIRE_EQ(a.count(5), 1);
    }

    SUBCASE("remove value")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } } });
        a.remove_value(1);
        a.remove_value(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.count(1), 2);
        REQUIRE_EQ(a.count(4), 1);
        REQUIRE_EQ(a.count(5), 1);

        a.append({ { 1, 114514 }, { 2, 114514 }, { 3, 114514 }, { 4, 114514 } });
        a.remove_all_value(114514);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.count(1), 2);
        REQUIRE_EQ(a.count(4), 1);
        REQUIRE_EQ(a.count(5), 1);
    }

    // [test in single map] remove if

    SUBCASE("find")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        {
            auto ref = a.find(1);
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 1);
            REQUIRE_EQ(ref.value(), 1);

            int64_t count = 0;
            while (ref)
            {
                ++count;
                ref = a.find_next(ref, 1);
            }
            REQUIRE_EQ(count, 3);
        }
        {
            auto ref = a.find_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 5);
            REQUIRE_EQ(ref.value(), 114514);

            int64_t count = 0;
            while (ref)
            {
                ++count;
                ref = a.find_next_ex(ref, [](const KeyType& key) { return key == 5; });
            }
            REQUIRE_EQ(count, 1);
        }
        {
            auto ref = a.readonly().find(1);
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 1);
            REQUIRE_EQ(ref.value(), 1);

            int64_t count = 0;
            while (ref)
            {
                ++count;
                ref = a.readonly().find_next(ref, 1);
            }
            REQUIRE_EQ(count, 3);
        }
        {
            auto ref = a.readonly().find_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 5);
            REQUIRE_EQ(ref.value(), 114514);

            int64_t count = 0;
            while (ref)
            {
                ++count;
                ref = a.readonly().find_next_ex(ref, [](const KeyType& key) { return key == 5; });
            }
            REQUIRE_EQ(count, 1);
        }
    }

    SUBCASE("find value")
    {
        TestHashMap a({ { { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } } });
        {
            auto ref = a.find_value(114514);
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 5);
            REQUIRE_EQ(ref.value(), 114514);
        }
        {
            auto ref = a.readonly().find_value(4);
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 4);
            REQUIRE_EQ(ref.value(), 4);
        }
    }

    SUBCASE("find if")
    {
        TestHashMap a({ { { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 6 } } });
        {
            auto ref = a.find_if([](auto&& pair) { return pair.key <= 3; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 1);
            REQUIRE_EQ(ref.value(), 1);
        }
        {
            auto ref = a.find_last_if([](auto&& pair) { return pair.key >= 4; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 6);
            REQUIRE_EQ(ref.value(), 6);
        }
        {
            auto ref = a.readonly().find_if([](auto&& pair) { return pair.key <= 3; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 1);
            REQUIRE_EQ(ref.value(), 1);
        }
        {
            auto ref = a.readonly().find_last_if([](auto&& pair) { return pair.key >= 4; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.key(), 6);
            REQUIRE_EQ(ref.value(), 6);
        }
        {
            auto ref = a.find_if([](auto&& pair) { return pair.key <= 0; });
            REQUIRE_FALSE(ref);
        }
        {
            auto ref = a.find_last_if([](auto&& pair) { return pair.key >= 7; });
            REQUIRE_FALSE(ref);
        }
        {
            auto ref = a.readonly().find_if([](auto&& pair) { return pair.key <= 0; });
            REQUIRE_FALSE(ref);
        }
        {
            auto ref = a.readonly().find_last_if([](auto&& pair) { return pair.key >= 7; });
            REQUIRE_FALSE(ref);
        }
    }

    SUBCASE("contains")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } });
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(114514));
        REQUIRE(a.contains_ex(Hash<KeyType>()(1), [](const KeyType& key) { return key == 1; }));
        REQUIRE(a.contains_ex(Hash<KeyType>()(4), [](const KeyType& key) { return key == 4; }));
        REQUIRE(a.contains_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; }));
        REQUIRE_FALSE(a.contains_ex(Hash<KeyType>()(114514), [](const KeyType& key) { return key == 114514; }));
    }

    SUBCASE("count")
    {
        TestHashMap a({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 114514 }, { 1, 1 }, { 4, 4 } });
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);
        REQUIRE_EQ(a.count(114514), 0);
        REQUIRE_EQ(a.count_ex(Hash<KeyType>()(1), [](const KeyType& key) { return key == 1; }), 3);
        REQUIRE_EQ(a.count_ex(Hash<KeyType>()(4), [](const KeyType& key) { return key == 4; }), 2);
        REQUIRE_EQ(a.count_ex(Hash<KeyType>()(5), [](const KeyType& key) { return key == 5; }), 1);
        REQUIRE_EQ(a.count_ex(Hash<KeyType>()(114514), [](const KeyType& key) { return key == 114514; }), 0);
    }

    // [test in sparse hash set] contains if & count if

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
            a.add(k, k * 5);
        }
        a.sort();
        REQUIRE_EQ(a.size(), 100);
        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(a.contains(i));
            REQUIRE(a.find(i).value() == i * 5);
            REQUIRE_EQ(a.data_vector()[i]._sparse_hash_set_data.key, i);
            REQUIRE_EQ(a.data_vector()[i]._sparse_hash_set_data.value, i * 5);
        }
    }

    SUBCASE("cursor & iter")
    {
        const auto kCapacity = clamp_capacity(114514);

        TestHashMap a(kCapacity);
        for (size_t i = 0; i < kCapacity; ++i)
        {
            a.add(i, i + 3);
        }
        for (size_t i = 0; i < kCapacity; ++i)
        {
            if (i % 2 == 1)
            {
                a.remove(i);
            }
        }

        auto test_func = [kCapacity](auto&& set) {
            uint64_t count;

            // iter
            count = 0;
            for (auto it = set.iter(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.key(), count * 2);
                REQUIRE_EQ(it.value(), it.key() + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto it = set.iter_inv(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.key(), (kCapacity / 2 - count - 1) * 2);
                REQUIRE_EQ(it.value(), it.key() + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // range
            count = 0;
            for (auto v : set.range())
            {
                REQUIRE_EQ(v.key, count * 2);
                REQUIRE_EQ(v.value, v.key + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto v : set.range_inv())
            {
                REQUIRE_EQ(v.key, (kCapacity / 2 - count - 1) * 2);
                REQUIRE_EQ(v.value, v.key + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // cursor
            count = 0;
            for (auto cur = set.cursor_begin(); !cur.reach_end(); cur.move_next())
            {
                REQUIRE_EQ(cur.key(), count * 2);
                REQUIRE_EQ(cur.value(), cur.key() + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto cur = set.cursor_end(); !cur.reach_begin(); cur.move_prev())
            {
                REQUIRE_EQ(cur.key(), (kCapacity / 2 - count - 1) * 2);
                REQUIRE_EQ(cur.value(), cur.key() + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // foreach
            count = 0;
            for (auto v : set)
            {
                REQUIRE_EQ(v.key, count * 2);
                REQUIRE_EQ(v.value, v.key + 3);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
        };
        test_func(a);
        test_func(a.readonly());
    }

    SUBCASE("empty container")
    {
        TestHashMap a;

        a.clear();
        a.release();
        a.reserve(0);
        a.shrink();
        a.compact();
        a.compact_stable();
        a.compact_top();
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.size(), 0);

        a.rehash_if_need();
        a.rehash();

        REQUIRE_FALSE(a.remove(114514));
        REQUIRE_FALSE(a.remove_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }));
        REQUIRE_EQ(a.remove_all(114514), 0);
        REQUIRE_EQ(a.remove_all_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }), 0);

        REQUIRE_FALSE(a.remove_value(114514));
        REQUIRE_EQ(a.remove_all_value(114514), 0);

        REQUIRE_FALSE(a.remove_if([](auto&& pair) { return pair.key == 114514; }));
        REQUIRE_FALSE(a.remove_last_if([](auto&& pair) { return pair.key == 114514; }));
        REQUIRE_EQ(a.remove_all_if([](auto&& pair) { return pair.key == 114514; }), 0);

        REQUIRE_FALSE((bool)a.find(114514));
        REQUIRE_FALSE((bool)a.find_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find(114514));
        REQUIRE_FALSE((bool)a.readonly().find_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }));

        {
            auto ref = a.find(114514);
            REQUIRE_FALSE((bool)a.find_next(ref, 114514));
            REQUIRE_FALSE((bool)a.readonly().find_next(ref, 114514));
            REQUIRE_FALSE((bool)a.find_next_ex(ref, [](const KeyType& v) { return v == 114514; }));
            REQUIRE_FALSE((bool)a.readonly().find_next_ex(ref, [](const KeyType& v) { return v == 114514; }));
        }

        REQUIRE_FALSE((bool)a.find_value(114514));
        REQUIRE_FALSE((bool)a.readonly().find_value(114514));

        REQUIRE_FALSE((bool)a.find_if([](auto&& pair) { return pair.value >= 114514; }));
        REQUIRE_FALSE((bool)a.find_last_if([](auto&& pair) { return pair.value >= 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_if([](auto&& pair) { return pair.value >= 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_last_if([](auto&& pair) { return pair.value >= 114514; }));

        REQUIRE_FALSE(a.contains(114514));
        REQUIRE_FALSE(a.contains_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }));
        REQUIRE_FALSE(a.readonly().contains(114514));
        REQUIRE_FALSE(a.readonly().contains_ex(Hash<KeyType>()(114514), [](const KeyType& v) { return v == 114514; }));

        REQUIRE_FALSE(a.contains_value(114514));
        REQUIRE_FALSE(a.readonly().contains_value(114514));

        REQUIRE_FALSE(a.contains_if([](auto&& pair) { return pair.key <= 114514; }));
        REQUIRE_FALSE(a.readonly().contains_if([](auto&& pair) { return pair.key >= 114514; }));

        REQUIRE_EQ(a.count_if([](auto&& pair) { return pair.key == 114514; }), 0);
        REQUIRE_EQ(a.count_if([](auto&& pair) { return pair.value == 114514; }), 0);
        REQUIRE_EQ(a.readonly().count_if([](auto&& pair) { return pair.key == 114514; }), 0);
        REQUIRE_EQ(a.readonly().count_if([](auto&& pair) { return pair.value == 114514; }), 0);

        // a.sort();
        // a.sort_stable();

        auto test_func = [](auto&& map) {
            uint64_t count;

            // iter
            count = 0;
            for (auto it = map.iter(); it.has_next(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto it = map.iter_inv(); it.has_next(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // range
            count = 0;
            for ([[maybe_unused]] auto n : map.range())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for ([[maybe_unused]] auto n : map.range_inv())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // cursor
            count = 0;
            for (auto cur = map.cursor_begin(); !cur.reach_end(); cur.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto cur = map.cursor_end(); !cur.reach_begin(); cur.move_prev())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // foreach
            count = 0;
            for ([[maybe_unused]] auto v : map)
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
        };
        test_func(a);
        test_func(a.readonly());
    }
}

TEST_CASE("test sparse hash map")
{
    using namespace skr::test_container;
    using KeyType   = int32_t;
    using ValueType = int32_t;
    using PairType  = skr::container::KVPair<KeyType, ValueType>;

    using TestHashMap = MultiSparseHashMap<KeyType, ValueType>;

    template_test_sparse_hash_map<KeyType, PairType, TestHashMap>(
        [](auto capacity) { return capacity; },
        [](auto capacity) { return capacity; }
    );
}

TEST_CASE("test fixed sparse hash map")
{
    using namespace skr::test_container;
    using KeyType   = int32_t;
    using ValueType = int32_t;
    using PairType  = skr::container::KVPair<KeyType, ValueType>;

    static constexpr size_t kFixedCapacity = 200;
    using TestHashMap                      = FixedMultiSparseHashMap<KeyType, ValueType, kFixedCapacity>;

    template_test_sparse_hash_map<KeyType, PairType, TestHashMap>(
        [](auto capacity) { return kFixedCapacity; },
        [](auto capacity) { return capacity < kFixedCapacity ? capacity : kFixedCapacity; }
    );
}

TEST_CASE("test inline sparse hash map")
{
    using namespace skr::test_container;
    using KeyType   = int32_t;
    using ValueType = int32_t;
    using PairType  = skr::container::KVPair<KeyType, ValueType>;

    static constexpr uint64_t kInlineCapacity = 10;

    using TestHashMap = InlineMultiSparseHashMap<KeyType, ValueType, kInlineCapacity>;
    template_test_sparse_hash_map<KeyType, PairType, TestHashMap>(
        [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
        [](auto capacity) { return capacity; }
    );

    SUBCASE("inline copy & move & assign & move assign")
    {
        using TestHashMap2 = InlineMultiSparseHashMap<KeyType, ValueType, 4>;

        TestHashMap2 map({ { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 } });

        TestHashMap2 map_copy{ map };
        TestHashMap2 map_move{ std::move(map) };
        TestHashMap2 map_assign;
        map_assign = map_copy;
        TestHashMap2 map_move_assign;
        map_move_assign = std::move(map_move);
    }
}