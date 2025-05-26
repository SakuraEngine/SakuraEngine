#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"
#include <chrono>

template <typename ValueType, typename TestHashSet, typename ModifyCapacity, typename ClampCapacity>
void template_test_sparse_hash_set(ModifyCapacity&& capacity_of, ClampCapacity&& clamp_capacity)
{
    using skr::Hash;
    using namespace skr::test_container;

    SUBCASE("ctor & dtor")
    {
        TestHashSet a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));

        TestHashSet b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(100));

        TestHashSet c({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), capacity_of(3));
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));

        int32_t     data[] = { 1, 1, 4, 5, 1, 4 };
        TestHashSet d(data, 6);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), capacity_of(3));
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
    }

    SUBCASE("copy & move")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.reserve(capacity_of(114514)); // test for bad capacity copy
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.sparse_size(), 3);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(3));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));

        TestHashSet b(a);
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(3));
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));

        auto        old_capacity = a.capacity();
        TestHashSet c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
    }

    SUBCASE("assign & move assign")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        TestHashSet b({ 114514, 114514, 1, 1, 4 });
        TestHashSet c({ 1, 1, 4, 514, 514, 514 });

        b = a;
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(3));
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));
        REQUIRE_FALSE(b.contains(114514));

        auto old_capacity = a.capacity();
        c                 = std::move(a);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(0));
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), capacity_of(old_capacity));
        REQUIRE(c.contains(1));
        REQUIRE(c.contains(4));
        REQUIRE(c.contains(5));
        REQUIRE_FALSE(c.contains(114514));
        REQUIRE_FALSE(a.contains(1));
    }

    SUBCASE("compare")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        TestHashSet b({ 114, 114, 514, 114, 514, 114 });
        TestHashSet c({ 1, 1, 4, 5, 1, 4 });
        TestHashSet d({ 1, 5, 4 });

        REQUIRE_EQ(a, c);
        REQUIRE_EQ(a, d);
        REQUIRE_EQ(c, d);

        REQUIRE_NE(a, b);
        REQUIRE_NE(b, c);
        REQUIRE_NE(b, d);

        a.remove(1);
        REQUIRE_NE(a, c);
        REQUIRE_NE(a, d);

        a.remove(4);
        a.remove(5);
        a.append({ 114, 514 });

        REQUIRE_EQ(a, b);
        REQUIRE_NE(a, c);
        REQUIRE_NE(a, d);
    }

    // [needn't test] getter
    // [needn't test] validate

    SUBCASE("memory op")
    {
        TestHashSet a({ 1, 11, 114, 1145, 11451, 114514 });
        a.remove(11);
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 1);
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

        // release eq to capcity
        a.reserve(capacity_of(4096));
        a.release(capacity_of(4096));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(4096));
        a.release();

        a.release(5);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(5));

        a.reserve(100);
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(100));

        a.append({ 1, 11, 114, 1145, 11451, 114514 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(100));

        a.remove(11451);
        a.remove(114514);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(4));

        a.clear();
        a.append({ 1, 11, 114, 1145, 11451, 114514 });
        a.remove(11451);
        a.remove(1);
        a.compact();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE(a.contains(11));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(1145));
        REQUIRE(a.contains(114514));

        a.clear();
        a.append({ 1, 11, 114, 1145, 11451, 114514 });
        a.remove(114);
        a.remove(11);
        a.compact_stable();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(1145));
        REQUIRE(a.contains(11451));
        REQUIRE(a.contains(114514));
    }

    // [needn't test] rehash

    SUBCASE("add")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.add(1);
        a.add(4);
        a.add(10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(4));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));

        auto find_result = a.find(1);
        REQUIRE(a.add(1, find_result).already_exist());
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(4));
        find_result = a.readonly().find(114514);
        REQUIRE_FALSE(a.add(114514, find_result).already_exist());
        REQUIRE(a.contains(114514));
        a.remove(114514);

        a.add_ex(
            Hash<ValueType>()(100),
            [](const ValueType& v) { return v == 100; },
            [](auto* p) { new (p) ValueType(100); },
            [](auto* p) { *p = 100; }
        );
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 5);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(5));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE(a.contains(100));

        auto ref = a.add_ex_unsafe(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; });
        new (ref.ptr()) ValueType(114514);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));
        REQUIRE(a.contains(100));
    }

    SUBCASE("emplace")
    {
        TestHashSet a({ { 1, 1, 4, 5, 1, 4 } });
        a.emplace(1);
        a.emplace(1);
        a.emplace(4);
        a.emplace(5);
        a.emplace(10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(4));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(10));

        auto find_result = a.find(1);
        REQUIRE(a.emplace(find_result, 1).already_exist());
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(4));
        find_result = a.readonly().find(114514);
        REQUIRE_FALSE(a.emplace(find_result, 114514).already_exist());
        REQUIRE(a.contains(114514));
        a.remove(114514);
    }

    SUBCASE("append")
    {
        TestHashSet a;
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.append({ 114514, 114514, 114514, 114, 514 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE(a.contains(114514));

        TestHashSet b;
        b.append(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(6));
        REQUIRE(b.contains(1));
        REQUIRE(b.contains(4));
        REQUIRE(b.contains(5));
        REQUIRE(b.contains(114));
        REQUIRE(b.contains(514));
        REQUIRE(b.contains(114514));
    }

    SUBCASE("remove")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.remove(1);
        a.remove(4);
        REQUIRE_EQ(a.size(), 1);
        REQUIRE_LE(a.sparse_size(), 3);
        REQUIRE_LE(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), capacity_of(3));
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(1));
        REQUIRE_FALSE(a.contains(4));

        a.append({ 114514, 114514, 114, 514 });
        a.remove_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; });
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE_FALSE(a.contains(114514));
    }

    SUBCASE("remove if")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.remove_if([](const ValueType& v) { return v == 1; });
        REQUIRE_EQ(a.size(), 2);
        REQUIRE_LE(a.sparse_size(), 3);
        REQUIRE_LE(a.hole_size(), 1);
        REQUIRE_GE(a.capacity(), capacity_of(3));
        REQUIRE(a.contains(5));
        REQUIRE_FALSE(a.contains(1));
        REQUIRE(a.contains(4));

        a.add(1);
        a.add(6);
        a.add(7);
        a.add(8);
        a.remove_last_if([](const ValueType& v) { return v > 5; });
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(6));
        REQUIRE(a.contains(7));
        REQUIRE_FALSE(a.contains(8));

        a.remove_all_if([](const ValueType& v) { return v % 2 == 1; });
        REQUIRE_FALSE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE_FALSE(a.contains(5));
        REQUIRE(a.contains(6));
        REQUIRE_FALSE(a.contains(7));
        REQUIRE_FALSE(a.contains(8));
    }

    SUBCASE("find")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        {
            auto ref = a.find(1);
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 1);
        }
        {
            auto ref = a.find_ex(Hash<ValueType>()(5), [](const ValueType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 5);
        }
        {
            auto ref = a.readonly().find(1);
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 1);
        }
        {
            auto ref = a.readonly().find_ex(Hash<ValueType>()(5), [](const ValueType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 5);
        }
    }

    SUBCASE("find if")
    {
        TestHashSet a({ 1, 2, 3, 4, 5, 6 });
        {
            auto ref = a.find_if([](const ValueType& v) { return v >= 1; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 1);
        }
        {
            auto ref = a.find_last_if([](const ValueType& key) { return key <= 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 5);
        }
        {
            auto ref = a.readonly().find_if([](const ValueType& v) { return v >= 1; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 1);
        }
        {
            auto ref = a.readonly().find_last_if([](const ValueType& key) { return key <= 5; });
            REQUIRE(ref);
            REQUIRE_EQ(ref.ref(), 5);
        }
    }

    SUBCASE("contains")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        REQUIRE(a.readonly().contains(1));
        REQUIRE(a.readonly().contains(4));
        REQUIRE(a.readonly().contains(5));
        REQUIRE_FALSE(a.contains(114514));
        REQUIRE(a.readonly().contains_ex(Hash<ValueType>()(1), [](const ValueType& key) { return key == 1; }));
        REQUIRE(a.readonly().contains_ex(Hash<ValueType>()(4), [](const ValueType& key) { return key == 4; }));
        REQUIRE(a.readonly().contains_ex(Hash<ValueType>()(5), [](const ValueType& key) { return key == 5; }));
        REQUIRE_FALSE(a.readonly().contains_ex(Hash<ValueType>()(114514), [](const ValueType& key) { return key == 114514; }));
    }

    SUBCASE("contains_if & count_if")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        REQUIRE(a.readonly().contains_if([](const ValueType v) { return v == 1; }));
        REQUIRE(a.readonly().contains_if([](const ValueType v) { return v == 4; }));
        REQUIRE(a.readonly().contains_if([](const ValueType v) { return v == 5; }));
        REQUIRE_FALSE(a.readonly().contains_if([](const ValueType v) { return v == 114514; }));

        REQUIRE_EQ(a.readonly().count_if([](const ValueType v) { return v == 1; }), 1);
        REQUIRE_EQ(a.readonly().count_if([](const ValueType v) { return v != 5; }), 2);
        REQUIRE_EQ(a.readonly().count_if([](const ValueType v) { return v > 1; }), 2);
    }

    // [needn't test] visitor & modifier

    SUBCASE("sort")
    {
        srand(std::chrono::system_clock::now().time_since_epoch().count());
        TestHashSet a(100);
        for (auto i = 0; i < 100; ++i)
        {
            auto k = rand() % 100;
            while (a.contains(k))
            {
                k = rand() % 100;
            }
            a.add(k);
        }
        a.sort();
        REQUIRE_EQ(a.size(), 100);
        REQUIRE_EQ(a.sparse_size(), 100);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), capacity_of(100));
        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(a.contains(i));
            REQUIRE_EQ(a.data_vector()[i]._sparse_hash_set_data, i);
        }
    }

    SUBCASE("set ops")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        TestHashSet b({ 1, 1, 4 });
        TestHashSet c({ 1, 11, 114, 1145, 11451, 114514 });

        TestHashSet intersect_a_b  = a & b;
        TestHashSet union_a_c      = a | c;
        TestHashSet difference_a_c = a ^ c;
        TestHashSet sub_a_c        = a - c;

        REQUIRE(b.is_sub_set_of(a));

        REQUIRE_EQ(intersect_a_b.size(), 2);
        REQUIRE(intersect_a_b == TestHashSet({ 1, 4 }));

        REQUIRE_EQ(union_a_c.size(), 8);
        REQUIRE(union_a_c == TestHashSet({ 1, 4, 5, 11, 114, 1145, 11451, 114514 }));

        REQUIRE_EQ(difference_a_c.size(), 7);
        REQUIRE(difference_a_c == TestHashSet({ 4, 5, 11, 114, 1145, 11451, 114514 }));

        REQUIRE_EQ(sub_a_c.size(), 2);
        REQUIRE(sub_a_c == TestHashSet({ 4, 5 }));
    }

    SUBCASE("cursor & iter")
    {
        const size_t kCapacity = clamp_capacity(114514);

        TestHashSet a(kCapacity);
        for (size_t i = 0; i < kCapacity; ++i)
        {
            a.add(i);
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
                REQUIRE_EQ(it.ref(), count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto it = set.iter_inv(); it.has_next(); it.move_next())
            {
                REQUIRE_EQ(it.ref(), (kCapacity / 2 - count - 1) * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // range
            count = 0;
            for (auto n : set.range())
            {
                REQUIRE_EQ(n, count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto n : set.range_inv())
            {
                REQUIRE_EQ(n, (kCapacity / 2 - count - 1) * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // cursor
            count = 0;
            for (auto cur = set.cursor_begin(); !cur.reach_end(); cur.move_next())
            {
                REQUIRE_EQ(cur.ref(), count * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);
            count = 0;
            for (auto cur = set.cursor_end(); !cur.reach_begin(); cur.move_prev())
            {
                REQUIRE_EQ(cur.ref(), (kCapacity / 2 - count - 1) * 2);
                ++count;
            }
            REQUIRE_EQ(count, kCapacity / 2);

            // foreach
            count = 0;
            for (auto v : set)
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
        TestHashSet a;

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
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.size(), 0);

        a.rehash_if_need();
        a.rehash();

        REQUIRE_FALSE(a.remove(114514));
        REQUIRE_FALSE(a.remove_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE(a.remove_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE(a.remove_last_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_EQ(a.remove_all_if([](const ValueType& v) { return v == 114514; }), 0);

        REQUIRE_FALSE((bool)a.find(114514));
        REQUIRE_FALSE((bool)a.find_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find(114514));
        REQUIRE_FALSE((bool)a.readonly().find_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE((bool)a.find_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.find_last_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_last_if([](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE(a.contains(114514));
        REQUIRE_FALSE(a.contains_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE(a.readonly().contains(114514));
        REQUIRE_FALSE(a.readonly().contains_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE(a.contains_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_EQ(a.count_if([](const ValueType& v) { return v == 114514; }), 0);
        REQUIRE_FALSE(a.readonly().contains_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_EQ(a.readonly().count_if([](const ValueType& v) { return v == 114514; }), 0);

        a.sort();
        a.sort_stable();

        {
            TestHashSet b, c, d;
            c = a & b;
            d = a | b;
            c = a ^ b;
            d = a - b;
            REQUIRE(a.is_sub_set_of(b));
        }

        auto test_func = [](auto&& set) {
            uint64_t count;

            // iter
            count = 0;
            for (auto it = set.iter(); it.has_next(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto it = set.iter_inv(); it.has_next(); it.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // range
            count = 0;
            for ([[maybe_unused]] auto n : set.range())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for ([[maybe_unused]] auto n : set.range_inv())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // cursor
            count = 0;
            for (auto cur = set.cursor_begin(); !cur.reach_end(); cur.move_next())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
            count = 0;
            for (auto cur = set.cursor_end(); !cur.reach_begin(); cur.move_prev())
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);

            // foreach
            count = 0;
            for ([[maybe_unused]] auto v : set)
            {
                ++count;
            }
            REQUIRE_EQ(count, 0);
        };
        test_func(a);
        test_func(a.readonly());
    }
}

TEST_CASE("test sparse hash set (Single)")
{
    using namespace skr::test_container;
    using ValueType   = int32_t;
    using TestHashSet = SparseHashSet<ValueType>;

    template_test_sparse_hash_set<ValueType, TestHashSet>(
        [](auto capacity) { return capacity; },
        [](auto capacity) { return capacity; }
    );
}

TEST_CASE("test fixed sparse hash set (Single)")
{
    using namespace skr::test_container;
    using ValueType                          = int32_t;
    static constexpr uint64_t kFixedCapacity = 200;
    using TestHashSet                        = FixedSparseHashSet<ValueType, kFixedCapacity>;

    template_test_sparse_hash_set<ValueType, TestHashSet>(
        [](auto capacity) { return kFixedCapacity; },
        [](auto capacity) { return capacity < kFixedCapacity ? capacity : kFixedCapacity; }
    );
}

TEST_CASE("test inline sparse hash set (Single)")
{
    using namespace skr::test_container;
    using ValueType = int32_t;

    static constexpr uint64_t kInlineCapacity = 10;

    using TestHashSet = InlineSparseHashSet<ValueType, kInlineCapacity>;

    template_test_sparse_hash_set<ValueType, TestHashSet>(
        [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
        [](auto capacity) { return capacity; }
    );

    SUBCASE("inline copy & move & assign & move assign")
    {
        InlineSparseHashSet<ValueType, 4> set({ 1, 2, 3, 4 });
        InlineSparseHashSet<ValueType, 4> set_copy{ set };
        InlineSparseHashSet<ValueType, 4> set_move{ std::move(set) };
        InlineSparseHashSet<ValueType, 4> set_assign;
        set_assign = set_copy;
        InlineSparseHashSet<ValueType, 4> set_move_assign;
        set_move_assign = std::move(set_move);
    }
}