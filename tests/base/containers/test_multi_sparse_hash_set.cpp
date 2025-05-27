#include "container_test_types.hpp"
#include "SkrTestFramework/framework.hpp"
#include <chrono>

template <typename ValueType, typename TestHashSet, typename ModifyCapacity, typename ClampCapacity>
void template_test_multi_sparse_hash_set(ModifyCapacity&& capacity_of, ClampCapacity&& clamp_capacity)
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
        REQUIRE_EQ(c.size(), 6);
        REQUIRE_EQ(c.sparse_size(), 6);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), capacity_of(6));
        REQUIRE_EQ(c.count(1), 3);
        REQUIRE_EQ(c.count(4), 2);
        REQUIRE_EQ(c.count(5), 1);

        int32_t     data[] = { 1, 1, 4, 5, 1, 4 };
        TestHashSet d(data, 6);
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
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.reserve(capacity_of(114514)); // test for bad capacity copy
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), capacity_of(6));
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);

        TestHashSet b(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), capacity_of(6));
        REQUIRE_EQ(b.count(1), 3);
        REQUIRE_EQ(b.count(4), 2);
        REQUIRE_EQ(b.count(5), 1);

        auto        old_capacity = a.capacity();
        TestHashSet c(std::move(a));
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
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        TestHashSet b({ 114514, 114514, 1, 1, 4 });
        TestHashSet c({ 1, 1, 4, 514, 514, 514 });

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

    // [test in single set] memory op
    // [needn't test] rehash

    SUBCASE("add")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.add(1);
        REQUIRE_EQ(a.size(), 7);
        a.add(114514);
        REQUIRE_EQ(a.size(), 8);
        REQUIRE_EQ(a.count(1), 4);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);
        REQUIRE_EQ(a.count(114514), 1);

        a.add_ex(Hash<ValueType>()(114514), [](void* p) { new (p) ValueType(114514); });
        REQUIRE_EQ(a.size(), 9);
        REQUIRE_EQ(a.count(1), 4);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);
        REQUIRE_EQ(a.count(114514), 2);

        auto result  = a.add_ex_unsafe(Hash<ValueType>()(5));
        result.ref() = 5;
        REQUIRE_EQ(a.size(), 10);
        REQUIRE_EQ(a.count(1), 4);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 2);
        REQUIRE_EQ(a.count(114514), 2);
    }

    SUBCASE("emplace")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.emplace(1);
        a.emplace(1);
        a.emplace(4);
        a.emplace(5);
        a.emplace(1);
        a.emplace(4);
        REQUIRE_EQ(a.size(), 12);
        REQUIRE_EQ(a.count(1), 6);
        REQUIRE_EQ(a.count(4), 4);
        REQUIRE_EQ(a.count(5), 2);
    }

    SUBCASE("append")
    {
        TestHashSet a;
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.append({ 114514, 114514, 114514, 114, 514 });
        REQUIRE_EQ(a.size(), 11);
        REQUIRE_EQ(a.count(1), 3);
        REQUIRE_EQ(a.count(4), 2);
        REQUIRE_EQ(a.count(5), 1);
        REQUIRE_EQ(a.count(114514), 3);
        REQUIRE_EQ(a.count(114), 1);

        TestHashSet b;
        b.append(a);
        REQUIRE_EQ(b.size(), 11);
        REQUIRE_EQ(b.count(1), 3);
        REQUIRE_EQ(b.count(4), 2);
        REQUIRE_EQ(b.count(5), 1);
        REQUIRE_EQ(b.count(114514), 3);
        REQUIRE_EQ(b.count(114), 1);
    }

    SUBCASE("remove")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.remove(1);
        a.remove(4);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));

        a.append({ 114514, 114514, 114, 514 });
        a.remove_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; });
        REQUIRE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE(a.contains(114514));

        a.remove_all(1);
        REQUIRE_FALSE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE(a.contains(114514));

        a.append({ 114514, 114514, 114514 });
        a.remove_all_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; });
        REQUIRE_FALSE(a.contains(1));
        REQUIRE(a.contains(4));
        REQUIRE(a.contains(5));
        REQUIRE(a.contains(114));
        REQUIRE(a.contains(514));
        REQUIRE_FALSE(a.contains(114514));
    }

    // [Test in single container] remove if

    SUBCASE("find")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.find(1).ref(), 1);
        REQUIRE_EQ(a.find(4).ref(), 4);
        REQUIRE_EQ(a.find(5).ref(), 5);
        REQUIRE_EQ(a.find(114514).is_valid(), false);

        REQUIRE_EQ(a.readonly().find(1).ref(), 1);
        REQUIRE_EQ(a.readonly().find(4).ref(), 4);
        REQUIRE_EQ(a.readonly().find(5).ref(), 5);
        REQUIRE_EQ(a.readonly().find(114514).is_valid(), false);

        REQUIRE_EQ(a.find_ex(Hash<ValueType>()(1), [](const ValueType& v) { return v == 1; }).ref(), 1);
        REQUIRE_EQ(a.find_ex(Hash<ValueType>()(4), [](const ValueType& v) { return v == 4; }).ref(), 4);
        REQUIRE_EQ(a.find_ex(Hash<ValueType>()(5), [](const ValueType& v) { return v == 5; }).ref(), 5);
        REQUIRE_EQ(a.find_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }).is_valid(), false);

        REQUIRE_EQ(a.readonly().find_ex(Hash<ValueType>()(1), [](const ValueType& v) { return v == 1; }).ref(), 1);
        REQUIRE_EQ(a.readonly().find_ex(Hash<ValueType>()(4), [](const ValueType& v) { return v == 4; }).ref(), 4);
        REQUIRE_EQ(a.readonly().find_ex(Hash<ValueType>()(5), [](const ValueType& v) { return v == 5; }).ref(), 5);
        REQUIRE_EQ(a.readonly().find_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }).is_valid(), false);

        {
            auto    result = a.find(1);
            int64_t count  = 0;
            while (result)
            {
                ++count;
                result = a.find_next(result, 1);
            };
            REQUIRE_EQ(count, 3);
        }
    }

    // [Test in single container] find if
    // [Test in single container] contains
    // [Test in single container] contains if
    // [needn't test] visitor & modifier
    // [Test in single container] sort

    SUBCASE("cursor & iter")
    {
        const auto kCapacity = clamp_capacity(114514);

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
        REQUIRE_FALSE(a.remove_all(114514));
        REQUIRE_FALSE(a.remove_all_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE(a.remove_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE(a.remove_last_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_EQ(a.remove_all_if([](const ValueType& v) { return v == 114514; }), 0);

        REQUIRE_FALSE((bool)a.find(114514));
        REQUIRE_FALSE((bool)a.find_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find(114514));
        REQUIRE_FALSE((bool)a.readonly().find_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));

        auto result = a.find(114514);
        REQUIRE_FALSE((bool)a.find_next(result, 114514));
        REQUIRE_FALSE((bool)a.find_next_ex(result, [](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_next(result, 114514));
        REQUIRE_FALSE((bool)a.readonly().find_next_ex(result, [](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE((bool)a.find_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.find_last_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE((bool)a.readonly().find_last_if([](const ValueType& v) { return v == 114514; }));

        REQUIRE_FALSE(a.contains(114514));
        REQUIRE_FALSE(a.contains_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));
        REQUIRE_FALSE(a.readonly().contains(114514));
        REQUIRE_FALSE(a.readonly().contains_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }));

        REQUIRE_EQ(a.count(114514), 0);
        REQUIRE_EQ(a.count_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }), 0);
        REQUIRE_EQ(a.readonly().count(114514), 0);
        REQUIRE_EQ(a.readonly().count_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; }), 0);

        REQUIRE_FALSE(a.contains_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_EQ(a.count_if([](const ValueType& v) { return v == 114514; }), 0);
        REQUIRE_FALSE(a.readonly().contains_if([](const ValueType& v) { return v == 114514; }));
        REQUIRE_EQ(a.readonly().count_if([](const ValueType& v) { return v == 114514; }), 0);

        a.sort();
        a.sort_stable();

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

TEST_CASE("test sparse hash set (multi)")
{
    using namespace skr::test_container;
    using ValueType   = int32_t;
    using TestHashSet = MultiSparseHashSet<ValueType>;

    template_test_multi_sparse_hash_set<ValueType, TestHashSet>(
        [](auto capacity) { return capacity; },
        [](auto capacity) { return capacity; }
    );
}

TEST_CASE("test fixed sparse hash set (multi)")
{
    using namespace skr::test_container;
    using ValueType                          = int32_t;
    static constexpr uint64_t kFixedCapacity = 200;
    using TestHashSet                        = FixedMultiSparseHashSet<ValueType, kFixedCapacity>;

    template_test_multi_sparse_hash_set<ValueType, TestHashSet>(
        [](auto capacity) { return kFixedCapacity; },
        [](auto capacity) { return capacity < kFixedCapacity ? capacity : kFixedCapacity; }
    );
}

TEST_CASE("test inline sparse hash set (multi)")
{
    using namespace skr::test_container;
    using ValueType = int32_t;

    static constexpr uint64_t kInlineCapacity = 10;

    using TestHashSet = InlineMultiSparseHashSet<ValueType, kInlineCapacity>;

    template_test_multi_sparse_hash_set<ValueType, TestHashSet>(
        [](auto capacity) { return capacity < kInlineCapacity ? kInlineCapacity : capacity; },
        [](auto capacity) { return capacity; }
    );

    SUBCASE("inline copy & move & assign & move assign")
    {
        using TestHashSet2 = InlineMultiSparseHashSet<ValueType, 4>;

        TestHashSet2 set({ 1, 2, 3, 4 });
        TestHashSet2 set_copy(set);
        TestHashSet2 set_move(std::move(set));
        TestHashSet2 set_assign;
        set_assign = set_copy;
        TestHashSet2 set_move_assign;
        set_move_assign = std::move(set_move);
    }
}