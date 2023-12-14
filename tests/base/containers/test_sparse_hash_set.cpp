#include "SkrTestFramework/framework.hpp"
#include "test_types.hpp"

TEST_CASE("test sparse hash set (Single)")
{
    using namespace skr;
    using ValueType   = int32_t;
    using TestHashSet = SparseHashSet<ValueType>;

    SUBCASE("ctor & dtor")
    {
        TestHashSet a;
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
        REQUIRE_EQ(a.data_arr().data(), nullptr);

        TestHashSet b(100);
        REQUIRE_EQ(b.size(), 0);
        REQUIRE_EQ(b.sparse_size(), 0);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 100);
        for (size_t i = 0; i < 100; ++i)
        {
            REQUIRE_FALSE(b.has_data(i));
        }

        TestHashSet c({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));

        int32_t     data[] = { 1, 1, 4, 5, 1, 4 };
        TestHashSet d(data, 6);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_GE(c.capacity(), 3);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));
    }

    SUBCASE("copy & move")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        REQUIRE_EQ(a.size(), 3);
        REQUIRE_EQ(a.sparse_size(), 3);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 3);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));

        TestHashSet b(a);
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 3);
        REQUIRE(b.contain(1));
        REQUIRE(b.contain(4));
        REQUIRE(b.contain(5));

        auto        old_capacity = a.capacity();
        TestHashSet c(std::move(a));
        REQUIRE_EQ(a.size(), 0);
        REQUIRE_EQ(a.sparse_size(), 0);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 0);
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
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        TestHashSet b({ 114514, 114514, 1, 1, 4 });
        TestHashSet c({ 1, 1, 4, 514, 514, 514 });

        b = a;
        REQUIRE_EQ(b.size(), 3);
        REQUIRE_EQ(b.sparse_size(), 3);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 3);
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
        REQUIRE_EQ(a.data_arr().data(), nullptr);
        REQUIRE_EQ(c.size(), 3);
        REQUIRE_EQ(c.sparse_size(), 3);
        REQUIRE_EQ(c.hole_size(), 0);
        REQUIRE_EQ(c.capacity(), old_capacity);
        REQUIRE(c.contain(1));
        REQUIRE(c.contain(4));
        REQUIRE(c.contain(5));
        REQUIRE_FALSE(c.contain(114514));
        REQUIRE_FALSE(a.contain(1));
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

        a.append({ 1, 11, 114, 1145, 11451, 114514 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 100);

        a.remove(11451);
        a.remove(114514);
        a.shrink();
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 6);

        a.clear();
        a.append({ 1, 11, 114, 1145, 11451, 114514 });
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
        a.append({ 1, 11, 114, 1145, 11451, 114514 });
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
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.add(1);
        a.add(4);
        a.add(10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));

        a.add_ex(
        Hash<ValueType>()(100),
        [](const ValueType& v) { return v == 100; },
        [](void* p) { new (p) ValueType(100); });
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 5);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 5);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));
        REQUIRE(a.contain(100));

        auto ref = a.add_ex_unsafe(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; });
        new (ref.data) ValueType(114514);
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));
        REQUIRE(a.contain(100));
    }

    SUBCASE("add or assign")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.add_or_assign(1);
        a.add_or_assign(4);
        a.add_or_assign(10);
        REQUIRE_EQ(a.size(), 4);
        REQUIRE_EQ(a.sparse_size(), 4);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));

        using container::KVPair;
        using TestAddOrAssignValue = KVPair<ValueType, ValueType>;
        using TestAddOrAssignSet   = container::SparseHashSet<container::SparseHashSetMemory<
        TestAddOrAssignValue,
        uint64_t,
        uint64_t,
        Hash<ValueType>,
        Equal<ValueType>,
        false,
        uint64_t,
        SkrTestAllocator_New>>;
        TestAddOrAssignSet b({ { 1, 1 }, { 1, 1 }, { 4, 4 }, { 5, 5 }, { 1, 1 }, { 4, 4 } });
        b.add_or_assign({ 1, 2 });
        b.add_or_assign({ 4, 5 });
        b.add_or_assign({ 5, 6 });
        b.add_or_assign({ 10, 10 });
        REQUIRE_EQ(b.size(), 4);
        REQUIRE_EQ(b.sparse_size(), 4);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 4);
        REQUIRE(b.contain(1));
        REQUIRE(b.contain(4));
        REQUIRE(b.contain(5));
        REQUIRE(b.contain(10));
        REQUIRE_EQ(b.find(1)->value, 2);
        REQUIRE_EQ(b.find(4)->value, 5);
        REQUIRE_EQ(b.find(5)->value, 6);
        REQUIRE_EQ(b.find(10)->value, 10);
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
        REQUIRE_GE(a.capacity(), 4);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));

        a.emplace_ex(
        Hash<ValueType>()(100),
        [](const ValueType& v) { return v == 100; },
        100);
        REQUIRE_EQ(a.size(), 5);
        REQUIRE_EQ(a.sparse_size(), 5);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 5);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(10));
        REQUIRE(a.contain(100));
    }

    SUBCASE("append")
    {
        TestHashSet a;
        a.append({ 1, 1, 4, 5, 1, 4 });
        a.append({ 114514, 114514, 114514, 114, 514 });
        REQUIRE_EQ(a.size(), 6);
        REQUIRE_EQ(a.sparse_size(), 6);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_GE(a.capacity(), 6);
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(114));
        REQUIRE(a.contain(514));
        REQUIRE(a.contain(114514));

        TestHashSet b;
        b.append(a);
        REQUIRE_EQ(b.size(), 6);
        REQUIRE_EQ(b.sparse_size(), 6);
        REQUIRE_EQ(b.hole_size(), 0);
        REQUIRE_GE(b.capacity(), 6);
        REQUIRE(b.contain(1));
        REQUIRE(b.contain(4));
        REQUIRE(b.contain(5));
        REQUIRE(b.contain(114));
        REQUIRE(b.contain(514));
        REQUIRE(b.contain(114514));
    }

    SUBCASE("remove")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        a.remove(1);
        a.remove(4);
        REQUIRE_EQ(a.size(), 1);
        REQUIRE_LE(a.sparse_size(), 3);
        REQUIRE_LE(a.hole_size(), 2);
        REQUIRE_GE(a.capacity(), 3);
        REQUIRE(a.contain(5));
        REQUIRE_FALSE(a.contain(1));
        REQUIRE_FALSE(a.contain(4));

        a.append({ 114514, 114514, 114, 514 });
        a.remove_ex(Hash<ValueType>()(114514), [](const ValueType& v) { return v == 114514; });
        REQUIRE(a.contain(5));
        REQUIRE(a.contain(114));
        REQUIRE(a.contain(514));
        REQUIRE_FALSE(a.contain(114514));
    }

    SUBCASE("erase")
    {
        TestHashSet a(100), b(100);
        for (int32_t i = 0; i < 100; ++i)
        {
            a.add(i);
            b.add(i);
        }

        for (auto it = a.begin(); it != a.end(); ++it)
        {
            if (*it % 3 == 0)
            {
                a.erase(it);
            }
        }

        const TestHashSet& cb = b;
        for (auto it = cb.begin(); it != cb.end(); ++it)
        {
            if (*it % 3 == 0)
            {
                b.erase(it);
            }
        }

        for (int32_t i = 0; i < 100; ++i)
        {
            if (i % 3 == 0)
            {
                REQUIRE_FALSE(a.contain(i));
                REQUIRE_FALSE(b.contain(i));
            }
            else
            {
                REQUIRE(a.contain(i));
                REQUIRE(b.contain(i));
            }
        }
    }

    SUBCASE("find")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        {
            auto ref = a.find(1);
            REQUIRE(ref);
            REQUIRE_EQ(*ref, 1);
        }
        {
            auto ref = a.find_ex(Hash<ValueType>()(5), [](const ValueType& key) { return key == 5; });
            REQUIRE(ref);
            REQUIRE_EQ(*ref, 5);
        }
    }

    SUBCASE("contain")
    {
        TestHashSet a({ 1, 1, 4, 5, 1, 4 });
        REQUIRE(a.contain(1));
        REQUIRE(a.contain(4));
        REQUIRE(a.contain(5));
        REQUIRE_FALSE(a.contain(114514));
        REQUIRE(a.contain_ex(Hash<ValueType>()(1), [](const ValueType& key) { return key == 1; }));
        REQUIRE(a.contain_ex(Hash<ValueType>()(4), [](const ValueType& key) { return key == 4; }));
        REQUIRE(a.contain_ex(Hash<ValueType>()(5), [](const ValueType& key) { return key == 5; }));
        REQUIRE_FALSE(a.contain_ex(Hash<ValueType>()(114514), [](const ValueType& key) { return key == 114514; }));
    }

    SUBCASE("sort")
    {
        srand(std::chrono::system_clock::now().time_since_epoch().count());
        TestHashSet a(100);
        for (auto i = 0; i < 100; ++i)
        {
            auto k = rand() % 100;
            while (a.contain(k))
            {
                k = rand() % 100;
            }
            a.add(k);
        }
        a.sort();
        REQUIRE_EQ(a.size(), 100);
        REQUIRE_EQ(a.sparse_size(), 100);
        REQUIRE_EQ(a.hole_size(), 0);
        REQUIRE_EQ(a.capacity(), 100);
        for (auto i = 0; i < 100; ++i)
        {
            REQUIRE(a.contain(i));
            REQUIRE_EQ(a.data_arr()[i]._sparse_hash_set_data, i);
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

    // test iterator
    SUBCASE("iterator")
    {
        TestHashSet a;
        for (auto n : a)
        {
            printf("%d\n", n);
        }
    }
}