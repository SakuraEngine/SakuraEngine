#include "SkrTestFramework/framework.hpp"

#include <SkrCore/memory/rc.hpp>

static uint64_t rc_base_count           = 0;
static uint64_t rc_derived_count        = 0;
static uint64_t rc_custom_deleter_count = 0;

struct TestRCBase {
    SKR_RC_IMPL()
public:
    TestRCBase()
    {
        ++rc_base_count;
    }
    virtual ~TestRCBase()
    {
        --rc_base_count;
    }
};

struct TestRCDerived : public TestRCBase {
public:
    TestRCDerived()
    {
        ++rc_derived_count;
    }
    ~TestRCDerived()
    {
        --rc_derived_count;
    }
};

struct TestCustomDeleterRC {
    SKR_RC_IMPL()

    inline void skr_rc_delete()
    {
        ++rc_custom_deleter_count;
        SkrDelete(this);
    }
};

TEST_CASE("Test RC")
{
    using namespace skr;

    SUBCASE("ctor & dtor")
    {
        RC<TestRCBase> empty_ctor{};
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(empty_ctor.get(), nullptr);
        REQUIRE_EQ(empty_ctor.ref_count(), 0);
        REQUIRE_EQ(empty_ctor.ref_count_weak(), 0);

        RC<TestRCBase> null_ctor{ nullptr };
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(null_ctor.get(), nullptr);
        REQUIRE_EQ(null_ctor.ref_count(), 0);
        REQUIRE_EQ(null_ctor.ref_count_weak(), 0);

        {
            RC<TestRCBase> pointer_ctor{ SkrNew<TestRCBase>() };
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_NE(pointer_ctor.get(), nullptr);
            REQUIRE_EQ(pointer_ctor.ref_count(), 1);
            REQUIRE_EQ(pointer_ctor.ref_count_weak(), 0);
        }

        {
            RCUnique<TestRCBase> unique{ SkrNew<TestRCBase>() };
            RC<TestRCBase>       rc{ std::move(unique) };
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_NE(rc.get(), nullptr);
            REQUIRE_EQ(rc.ref_count(), 1);
            REQUIRE_EQ(rc.ref_count_weak(), 0);

            REQUIRE_EQ(unique.get(), nullptr);
        }

        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
    }

    SUBCASE("copy & move")
    {
        RC<TestRCBase>    base{ SkrNew<TestRCBase>() };
        RC<TestRCDerived> derived{ SkrNew<TestRCDerived>() };

        RC<TestRCBase> copy_base{ base };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.get(), copy_base.get());
        REQUIRE_EQ(base.ref_count(), 2);
        REQUIRE_EQ(copy_base.ref_count(), 2);
        REQUIRE_EQ(base.ref_count_weak(), 0);
        REQUIRE_EQ(copy_base.ref_count_weak(), 0);

        RC<TestRCBase> copy_derived{ derived };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(derived.get(), copy_derived.get());
        REQUIRE_EQ(derived.ref_count(), 2);
        REQUIRE_EQ(copy_derived.ref_count(), 2);
        REQUIRE_EQ(derived.ref_count_weak(), 0);
        REQUIRE_EQ(copy_derived.ref_count_weak(), 0);

        RC<TestRCBase> move_base{ std::move(base) };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.get(), nullptr);
        REQUIRE_EQ(move_base.get(), copy_base.get());
        REQUIRE_EQ(base.ref_count(), 0);
        REQUIRE_EQ(copy_base.ref_count(), 2);
        REQUIRE_EQ(move_base.ref_count(), 2);
        REQUIRE_EQ(base.ref_count_weak(), 0);
        REQUIRE_EQ(copy_base.ref_count_weak(), 0);

        RC<TestRCBase> move_derived{ std::move(derived) };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(derived.get(), nullptr);
        REQUIRE_EQ(move_derived.get(), copy_derived.get());
        REQUIRE_EQ(derived.ref_count(), 0);
        REQUIRE_EQ(copy_derived.ref_count(), 2);
        REQUIRE_EQ(move_derived.ref_count(), 2);
        REQUIRE_EQ(derived.ref_count_weak(), 0);
        REQUIRE_EQ(copy_derived.ref_count_weak(), 0);

        copy_base    = nullptr;
        move_base    = nullptr;
        copy_derived = nullptr;
        move_derived = nullptr;

        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
    }

    SUBCASE("assign & move assign")
    {
        RC<TestRCBase>    base{ SkrNew<TestRCBase>() };
        RC<TestRCDerived> derived{ SkrNew<TestRCDerived>() };

        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.ref_count(), 1);
        REQUIRE_EQ(derived.ref_count(), 1);
        {
            // assign
            RC<TestRCBase> assign{};
            assign = base;
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 2);
            REQUIRE_EQ(derived.ref_count(), 1);
            REQUIRE_EQ(assign.ref_count(), 2);
            REQUIRE_EQ(assign.get(), base.get());

            // assign derived
            assign = derived;
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 1);
            REQUIRE_EQ(derived.ref_count(), 2);
            REQUIRE_EQ(assign.ref_count(), 2);
            REQUIRE_EQ(assign.get(), derived.get());
        }

        {
            // move assign
            RC<TestRCBase> move_assign{};
            auto           cached_base = base.get();
            move_assign                = std::move(base);
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 0);
            REQUIRE_EQ(derived.ref_count(), 1);
            REQUIRE_EQ(move_assign.ref_count(), 1);
            REQUIRE_EQ(move_assign.get(), cached_base);
            REQUIRE_EQ(base.get(), nullptr);

            // move assign derived
            auto cached_derived = derived.get();
            move_assign         = std::move(derived);
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 0);
            REQUIRE_EQ(derived.ref_count(), 0);
            REQUIRE_EQ(move_assign.ref_count(), 1);
            REQUIRE_EQ(move_assign.get(), cached_derived);
            REQUIRE_EQ(derived.get(), nullptr);
        }

        {
            RCUnique<TestRCDerived> unique{ SkrNew<TestRCDerived>() };
            RC<TestRCBase>          move_assign_unique{};
            auto                    cached_unique = unique.get();
            move_assign_unique                    = std::move(unique);
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 0);
            REQUIRE_EQ(derived.ref_count(), 0);
            REQUIRE_EQ(move_assign_unique.ref_count(), 1);
            REQUIRE_EQ(move_assign_unique.get(), cached_unique);
            REQUIRE_EQ(derived.get(), nullptr);
        }
    }

    // [need't test] factory

    SUBCASE("test compare")
    {
        RC<TestRCBase>    base1{ SkrNew<TestRCBase>() };
        RC<TestRCBase>    base2{ SkrNew<TestRCBase>() };
        RC<TestRCDerived> derived1{ SkrNew<TestRCDerived>() };
        RC<TestRCDerived> derived2{ SkrNew<TestRCDerived>() };

        RC<TestRCBase> copy_base1    = base1;
        RC<TestRCBase> copy_derived1 = derived1;

        // eq
        REQUIRE_EQ(base1 == base1, true);
        REQUIRE_EQ(base1 == base2, false);
        REQUIRE_EQ(base1 == derived1, false);
        REQUIRE_EQ(base1 == copy_base1, true);
        REQUIRE_EQ(derived1 == copy_derived1, true);
        REQUIRE_EQ(derived2 == copy_derived1, false);

        // neq
        REQUIRE_EQ(base1 != base1, false);
        REQUIRE_EQ(base1 != base2, true);
        REQUIRE_EQ(base1 != derived1, true);
        REQUIRE_EQ(base1 != copy_base1, false);
        REQUIRE_EQ(derived1 != copy_derived1, false);
        REQUIRE_EQ(derived2 != copy_derived1, true);

        // nullptr
        REQUIRE_EQ(base1 == nullptr, false);
        REQUIRE_EQ(nullptr == base1, false);
        REQUIRE_EQ(base1 != nullptr, true);
        REQUIRE_EQ(nullptr != base1, true);

        // lt (just test for compile)
        REQUIRE_EQ(base1 < base1, false);
        REQUIRE_EQ(derived1 < copy_derived1, false);
        REQUIRE_EQ(copy_derived1 < derived1, false);

        // gt (just test for compile)
        REQUIRE_EQ(base1 > base1, false);
        REQUIRE_EQ(derived1 > copy_derived1, false);
        REQUIRE_EQ(copy_derived1 > derived1, false);

        // le (just test for compile)
        REQUIRE_EQ(base1 <= base1, true);
        REQUIRE_EQ(derived1 <= copy_derived1, true);
        REQUIRE_EQ(copy_derived1 <= derived1, true);

        // ge (just test for compile)
        REQUIRE_EQ(base1 >= base1, true);
        REQUIRE_EQ(derived1 >= copy_derived1, true);
        REQUIRE_EQ(copy_derived1 >= derived1, true);
    }

    // [need't test] getter
    // [need't test] count getter

    SUBCASE("test is empty")
    {
        RC<TestRCBase> empty{};
        RC<TestRCBase> something{ SkrNew<TestRCBase>() };

        REQUIRE_EQ(empty.is_empty(), true);
        REQUIRE_EQ(something.is_empty(), false);
        REQUIRE_EQ(empty, false);
        REQUIRE_EQ(something, true);
    }

    SUBCASE("test ops")
    {
        RC<TestRCBase> rc = SkrNew<TestRCBase>();
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(rc.get(), nullptr);

        rc.reset();
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(rc.get(), nullptr);

        rc.reset(SkrNew<TestRCBase>());
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(rc.get(), nullptr);

        RC<TestRCBase> other;
        rc.swap(other);
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(rc.get(), nullptr);
        REQUIRE_NE(other.get(), nullptr);

        other.reset(SkrNew<TestRCBase>());
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(other.get(), nullptr);
    }

    // [need't test] pointer behaviour
    // [need't test] skr hash

    SUBCASE("test empty")
    {
        RC<TestRCBase> empty{};
        RC<TestRCBase> empty_copy{ empty };
        RC<TestRCBase> empty_move{ std::move(empty) };
        RC<TestRCBase> empty_assign, empty_move_assign;
        empty_assign      = empty;
        empty_move_assign = std::move(empty);

        empty.get();
        empty.ref_count();
        empty.ref_count_weak();
        empty.is_empty();
        empty.operator bool();
        empty.reset();
        empty.reset(nullptr);
        empty.swap(empty_copy);
    }
}

TEST_CASE("Test RCUnique")
{
    using namespace skr;

    SUBCASE("ctor & dtor")
    {
        RCUnique<TestRCBase> empty_ctor{};
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(empty_ctor.get(), nullptr);
        REQUIRE_EQ(empty_ctor.ref_count(), 0);
        REQUIRE_EQ(empty_ctor.ref_count_weak(), 0);

        RCUnique<TestRCBase> null_ctor{ nullptr };
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(null_ctor.get(), nullptr);
        REQUIRE_EQ(null_ctor.ref_count(), 0);
        REQUIRE_EQ(null_ctor.ref_count_weak(), 0);

        {
            RCUnique<TestRCBase> pointer_ctor{ SkrNew<TestRCBase>() };
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_NE(pointer_ctor.get(), nullptr);
            REQUIRE_EQ(pointer_ctor.ref_count(), 1);
            REQUIRE_EQ(pointer_ctor.ref_count_weak(), 0);
        }

        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
    }

    SUBCASE("copy & move")
    {
        RCUnique<TestRCBase>    base{ SkrNew<TestRCBase>() };
        RCUnique<TestRCDerived> derived{ SkrNew<TestRCDerived>() };

        auto                 cached_base = base.get();
        RCUnique<TestRCBase> move_base{ std::move(base) };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.get(), nullptr);
        REQUIRE_EQ(base.ref_count(), 0);
        REQUIRE_EQ(move_base.ref_count(), 1);
        REQUIRE_EQ(move_base.get(), cached_base);

        auto                 cached_derived = derived.get();
        RCUnique<TestRCBase> move_derived{ std::move(derived) };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(derived.get(), nullptr);
        REQUIRE_EQ(move_derived.ref_count(), 1);
        REQUIRE_EQ(move_derived.get(), cached_derived);

        move_base    = nullptr;
        move_derived = nullptr;

        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
    }

    SUBCASE("assign & move assign")
    {
        RCUnique<TestRCBase>    base{ SkrNew<TestRCBase>() };
        RCUnique<TestRCDerived> derived{ SkrNew<TestRCDerived>() };

        RCUnique<TestRCBase> move_assign{};

        // move assign base
        auto cached_base = base.get();
        move_assign      = std::move(base);
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.get(), nullptr);
        REQUIRE_EQ(base.ref_count(), 0);
        REQUIRE_EQ(move_assign.ref_count(), 1);
        REQUIRE_EQ(move_assign.get(), cached_base);

        // move assign derived
        auto cached_derived = derived.get();
        move_assign         = std::move(derived);
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(derived.get(), nullptr);
        REQUIRE_EQ(derived.ref_count(), 0);
        REQUIRE_EQ(move_assign.ref_count(), 1);
        REQUIRE_EQ(move_assign.get(), cached_derived);
        REQUIRE_EQ(derived.get(), nullptr);

        move_assign = nullptr;
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(move_assign.get(), nullptr);
        REQUIRE_EQ(move_assign.ref_count(), 0);
    }

    SUBCASE("test compare")
    {
        RCUnique<TestRCBase>    base1{ SkrNew<TestRCBase>() };
        RCUnique<TestRCBase>    base2{ SkrNew<TestRCBase>() };
        RCUnique<TestRCDerived> derived1{ SkrNew<TestRCDerived>() };
        RCUnique<TestRCDerived> derived2{ SkrNew<TestRCDerived>() };

        // eq
        REQUIRE_EQ(base1 == base1, true);
        REQUIRE_EQ(base1 == base2, false);
        REQUIRE_EQ(base1 == derived1, false);

        // neq
        REQUIRE_EQ(base1 != base1, false);
        REQUIRE_EQ(base1 != base2, true);
        REQUIRE_EQ(base1 != derived1, true);

        // nullptr
        REQUIRE_EQ(base1 == nullptr, false);
        REQUIRE_EQ(nullptr == base1, false);
        REQUIRE_EQ(base1 != nullptr, true);
        REQUIRE_EQ(nullptr != base1, true);

        // lt (just test for compile)
        REQUIRE_EQ(base1 < base1, false);

        // gt (just test for compile)
        REQUIRE_EQ(base1 > base1, false);

        // le (just test for compile)
        REQUIRE_EQ(base1 <= base1, true);

        // ge (just test for compile)
        REQUIRE_EQ(base1 >= base1, true);
    }

    // [need't test] getter
    // [need't test] count getter

    SUBCASE("test is empty")
    {
        RCUnique<TestRCBase> empty{};
        RCUnique<TestRCBase> something{ SkrNew<TestRCBase>() };

        REQUIRE_EQ(empty.is_empty(), true);
        REQUIRE_EQ(something.is_empty(), false);
        REQUIRE_EQ(empty, false);
        REQUIRE_EQ(something, true);
    }

    SUBCASE("test ops")
    {
        RCUnique<TestRCBase> rc = SkrNew<TestRCBase>();
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(rc.get(), nullptr);

        rc.reset();
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(rc.get(), nullptr);

        rc.reset(SkrNew<TestRCBase>());
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(rc.get(), nullptr);

        RCUnique<TestRCBase> other;
        rc.swap(other);
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(rc.get(), nullptr);
        REQUIRE_NE(other.get(), nullptr);

        other.reset(SkrNew<TestRCBase>());
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(other.get(), nullptr);

        auto* cached_other   = other.get();
        auto* released_other = other.release();
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_EQ(other.get(), nullptr);
        REQUIRE_EQ(released_other, cached_other);
        SkrDelete(released_other);
    }

    // [need't test] pointer behaviour
    // [need't test] skr hash

    SUBCASE("test empty")
    {
        RCUnique<TestRCBase> empty{};
        RCUnique<TestRCBase> empty_move{ std::move(empty) };
        RCUnique<TestRCBase> empty_move_assign;
        empty_move_assign = std::move(empty);

        empty.get();
        empty.ref_count();
        empty.ref_count_weak();
        empty.is_empty();
        empty.operator bool();
        empty.reset();
        empty.reset(nullptr);
        empty.swap(empty_move);
        empty.release();
    }
}

TEST_CASE("Test RCWeak")
{
    using namespace skr;

    SUBCASE("test ctor")
    {
        RCWeak<TestRCBase> empty_ctor{};
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE(empty_ctor.lock().is_empty());
        REQUIRE_EQ(empty_ctor.ref_count_weak(), 0);

        RCWeak<TestRCBase> null_ctor{ nullptr };
        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE(null_ctor.lock().is_empty());
        REQUIRE_EQ(null_ctor.ref_count_weak(), 0);

        // from raw ptr
        {
            auto               rc_unique = RCUnique<TestRCBase>::New();
            RCWeak<TestRCBase> rc_weak{ rc_unique.get() };
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_FALSE(rc_weak.lock().is_empty());
            REQUIRE_EQ(rc_weak.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count(), 1);
            if (auto locked = rc_weak.lock())
            {
                REQUIRE_EQ(rc_base_count, 1);
                REQUIRE_EQ(rc_derived_count, 0);
                REQUIRE_EQ(locked.get(), rc_unique.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        // from raw ptr derived
        {
            auto               rc_unique = RCUnique<TestRCDerived>::New();
            RCWeak<TestRCBase> rc_weak{ rc_unique.get() };
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_FALSE(rc_weak.lock().is_empty());
            REQUIRE_EQ(rc_weak.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count(), 1);
            if (auto locked = rc_weak.lock())
            {
                REQUIRE_EQ(rc_base_count, 1);
                REQUIRE_EQ(rc_derived_count, 1);
                REQUIRE_EQ(locked.get(), rc_unique.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        // from rc
        {
            auto rc      = RC<TestRCBase>::New();
            auto rc_weak = RCWeak<TestRCBase>(rc);
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_FALSE(rc_weak.lock().is_empty());
            REQUIRE_EQ(rc_weak.ref_count_weak(), 2);
            REQUIRE_EQ(rc.ref_count_weak(), 2);
            REQUIRE_EQ(rc.ref_count(), 1);
            if (auto locked = rc_weak.lock())
            {
                REQUIRE_EQ(rc_base_count, 1);
                REQUIRE_EQ(rc_derived_count, 0);
                REQUIRE_EQ(locked.get(), rc.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        // from rc derived
        {
            auto rc      = RC<TestRCDerived>::New();
            auto rc_weak = RCWeak<TestRCBase>(rc);
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_FALSE(rc_weak.lock().is_empty());
            REQUIRE_EQ(rc_weak.ref_count_weak(), 2);
            REQUIRE_EQ(rc.ref_count_weak(), 2);
            REQUIRE_EQ(rc.ref_count(), 1);
            if (auto locked = rc_weak.lock())
            {
                REQUIRE_EQ(rc_base_count, 1);
                REQUIRE_EQ(rc_derived_count, 1);
                REQUIRE_EQ(locked.get(), rc.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        // from unique
        {
            auto rc_unique = RCUnique<TestRCBase>::New();
            auto rc_weak   = RCWeak<TestRCBase>(rc_unique);
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_FALSE(rc_weak.lock().is_empty());
            REQUIRE_EQ(rc_weak.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count(), 1);
            if (auto locked = rc_weak.lock())
            {
                REQUIRE_EQ(rc_base_count, 1);
                REQUIRE_EQ(rc_derived_count, 0);
                REQUIRE_EQ(locked.get(), rc_unique.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        // from unique derived
        {
            auto rc_unique = RCUnique<TestRCDerived>::New();
            auto rc_weak   = RCWeak<TestRCBase>(rc_unique);
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_FALSE(rc_weak.lock().is_empty());
            REQUIRE_EQ(rc_weak.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count_weak(), 2);
            REQUIRE_EQ(rc_unique.ref_count(), 1);
            if (auto locked = rc_weak.lock())
            {
                REQUIRE_EQ(rc_base_count, 1);
                REQUIRE_EQ(rc_derived_count, 1);
                REQUIRE_EQ(locked.get(), rc_unique.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
    }

    SUBCASE("copy & move")
    {
        auto base    = RCUnique<TestRCBase>::New();
        auto derived = RCUnique<TestRCDerived>::New();

        RCWeak<TestRCBase>    base_weak{ base };
        RCWeak<TestRCDerived> derived_weak{ derived };

        RCWeak<TestRCBase> copy_base{ base_weak };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base_weak.ref_count_weak(), 3);
        REQUIRE_EQ(copy_base.ref_count_weak(), 3);
        REQUIRE_EQ(copy_base.lock().get(), base_weak.lock().get());

        RCWeak<TestRCBase> copy_derived{ derived_weak };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(derived_weak.ref_count_weak(), 3);
        REQUIRE_EQ(copy_derived.ref_count_weak(), 3);
        REQUIRE_EQ(copy_derived.lock().get(), derived_weak.lock().get());

        RCWeak<TestRCBase> move_base{ std::move(base_weak) };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.ref_count_weak(), 3);
        REQUIRE_EQ(move_base.ref_count_weak(), 3);
        REQUIRE_EQ(move_base.lock().get(), base.get());

        RCWeak<TestRCBase> move_derived{ std::move(derived_weak) };
        REQUIRE_EQ(rc_base_count, 2);
        REQUIRE_EQ(rc_derived_count, 1);
        REQUIRE_EQ(base.ref_count_weak(), 3);
        REQUIRE_EQ(move_derived.ref_count_weak(), 3);
        REQUIRE_EQ(move_derived.lock().get(), derived.get());

        base         = nullptr;
        derived      = nullptr;
        base_weak    = nullptr;
        derived_weak = nullptr;
        copy_base    = nullptr;
        copy_derived = nullptr;
        move_base    = nullptr;
        move_derived = nullptr;

        REQUIRE_EQ(rc_base_count, 0);
        REQUIRE_EQ(rc_derived_count, 0);
    }

    SUBCASE("assign & move assign")
    {
        auto base    = RCUnique<TestRCBase>::New();
        auto derived = RC<TestRCDerived>::New();

        RCWeak<TestRCBase>    base_weak{ base };
        RCWeak<TestRCDerived> derived_weak{ derived };

        // assign weak
        {
            RCWeak<TestRCBase> assign{};

            // assign
            assign = base_weak;
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base_weak.ref_count_weak(), 3);
            REQUIRE_EQ(assign.ref_count_weak(), 3);
            REQUIRE_EQ(assign.lock().get(), base_weak.lock().get());

            // assign derived
            assign = derived_weak;
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(derived_weak.ref_count_weak(), 3);
            REQUIRE_EQ(assign.ref_count_weak(), 3);
            REQUIRE_EQ(assign.lock().get(), derived_weak.lock().get());

            REQUIRE_EQ(base_weak.ref_count_weak(), 2);
        }

        // move assign weak
        {
            RCWeak<TestRCBase> move_assign{};

            // move assign base
            auto cached_base = base_weak.lock().get();
            move_assign      = std::move(base_weak);
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE(base_weak.is_empty());
            REQUIRE_EQ(move_assign.ref_count_weak(), 2);
            REQUIRE_EQ(move_assign.lock().get(), cached_base);

            // move assign derived
            auto cached_derived = derived_weak.lock().get();
            move_assign         = std::move(derived_weak);
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE(derived_weak.is_empty());
            REQUIRE_EQ(move_assign.ref_count_weak(), 2);
            REQUIRE_EQ(move_assign.lock().get(), cached_derived);

            REQUIRE_EQ(base_weak.ref_count_weak(), 0);
        }

        // assign rc & unique rc
        {
            RCWeak<TestRCBase> assign{};
            assign = base;
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(base.ref_count_weak(), 2);
            REQUIRE_EQ(assign.ref_count_weak(), 2);
            REQUIRE_EQ(assign.lock().get(), base.get());

            // assign derived
            assign = derived;
            REQUIRE_EQ(rc_base_count, 2);
            REQUIRE_EQ(rc_derived_count, 1);
            REQUIRE_EQ(derived.ref_count_weak(), 2);
            REQUIRE_EQ(assign.ref_count_weak(), 2);
            REQUIRE_EQ(assign.lock().get(), derived.get());
        }
    }

    SUBCASE("test compare")
    {
        RC<TestRCBase>    rc_base1{ SkrNew<TestRCBase>() };
        RC<TestRCBase>    rc_base2{ SkrNew<TestRCBase>() };
        RC<TestRCDerived> rc_derived1{ SkrNew<TestRCDerived>() };
        RC<TestRCDerived> rc_derived2{ SkrNew<TestRCDerived>() };

        RCWeak<TestRCBase>    base1{ rc_base1 };
        RCWeak<TestRCBase>    base2{ rc_base2 };
        RCWeak<TestRCDerived> derived1{ rc_derived1 };
        RCWeak<TestRCDerived> derived2{ rc_derived2 };

        RCWeak<TestRCBase> copy_base1    = base1;
        RCWeak<TestRCBase> copy_derived1 = derived1;

        // eq
        REQUIRE_EQ(base1 == base1, true);
        REQUIRE_EQ(base1 == base2, false);
        REQUIRE_EQ(base1 == derived1, false);
        REQUIRE_EQ(base1 == copy_base1, true);
        REQUIRE_EQ(derived1 == copy_derived1, true);
        REQUIRE_EQ(derived2 == copy_derived1, false);

        // neq
        REQUIRE_EQ(base1 != base1, false);
        REQUIRE_EQ(base1 != base2, true);
        REQUIRE_EQ(base1 != derived1, true);
        REQUIRE_EQ(base1 != copy_base1, false);
        REQUIRE_EQ(derived1 != copy_derived1, false);
        REQUIRE_EQ(derived2 != copy_derived1, true);

        // nullptr
        REQUIRE_EQ(base1 == nullptr, false);
        REQUIRE_EQ(nullptr == base1, false);
        REQUIRE_EQ(base1 != nullptr, true);
        REQUIRE_EQ(nullptr != base1, true);

        // lt (just test for compile)
        REQUIRE_EQ(base1 < base1, false);
        REQUIRE_EQ(derived1 < copy_derived1, false);
        REQUIRE_EQ(copy_derived1 < derived1, false);

        // gt (just test for compile)
        REQUIRE_EQ(base1 > base1, false);
        REQUIRE_EQ(derived1 > copy_derived1, false);
        REQUIRE_EQ(copy_derived1 > derived1, false);

        // le (just test for compile)
        REQUIRE_EQ(base1 <= base1, true);
        REQUIRE_EQ(derived1 <= copy_derived1, true);
        REQUIRE_EQ(copy_derived1 <= derived1, true);

        // ge (just test for compile)
        REQUIRE_EQ(base1 >= base1, true);
        REQUIRE_EQ(derived1 >= copy_derived1, true);
        REQUIRE_EQ(copy_derived1 >= derived1, true);
    }

    // [need't test] unsafe getter
    // [need't test] count getter

    SUBCASE("lock")
    {
        RC<TestRCBase> rc{ SkrNew<TestRCBase>() };

        // basic lock
        RCWeak<TestRCBase> weak{ rc };
        REQUIRE(weak.is_alive());
        REQUIRE_FALSE(weak.is_expired());
        REQUIRE_FALSE(weak.lock().is_empty());
        {
            auto locked = weak.lock();
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_EQ(locked.get(), rc.get());
        }

        // lock a destroyed weak
        rc.reset();
        REQUIRE_FALSE(weak.is_alive());
        REQUIRE(weak.is_expired());
        REQUIRE(weak.lock().is_empty());
        {
            auto locked = weak.lock();
            REQUIRE_EQ(rc_base_count, 0);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_EQ(locked.get(), nullptr);
        }

        // try lock a destroyed weak to rc
        {
            RC<TestRCBase> locked_rc = weak.lock().rc();
            REQUIRE_EQ(rc_base_count, 0);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_EQ(locked_rc.get(), nullptr);
        }

        // lock and cast to rc
        rc   = SkrNew<TestRCBase>();
        weak = rc;
        REQUIRE(weak.is_alive());
        REQUIRE_FALSE(weak.is_expired());
        REQUIRE_FALSE(weak.lock().is_empty());
        {
            RC<TestRCBase> locked_rc = weak.lock();
            REQUIRE_EQ(rc_base_count, 1);
            REQUIRE_EQ(rc_derived_count, 0);
            REQUIRE_EQ(locked_rc.get(), rc.get());
            REQUIRE_EQ(locked_rc.ref_count(), 2);
        }
    }

    // [need't test] is empty

    SUBCASE("test ops")
    {
        RC<TestRCBase> rc = SkrNew<TestRCBase>();
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_NE(rc.get(), nullptr);

        RCWeak<TestRCBase> weak{ rc };
        REQUIRE_EQ(rc_base_count, 1);
        REQUIRE_EQ(rc_derived_count, 0);
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), rc.get());

        weak.reset();
        REQUIRE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 0);
        REQUIRE_EQ(weak.get_unsafe(), nullptr);
        REQUIRE_EQ(rc.ref_count_weak(), 1);

        weak.reset(rc);
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), rc.get());
        REQUIRE_EQ(rc.ref_count_weak(), 2);

        RCUnique<TestRCBase> new_rc = SkrNew<TestRCBase>();
        weak.reset(new_rc);
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), new_rc.get());
        REQUIRE_EQ(rc.ref_count_weak(), 1);
        REQUIRE_EQ(new_rc.ref_count_weak(), 2);

        weak.reset(rc.get());
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), rc.get());
        REQUIRE_EQ(rc.ref_count_weak(), 2);
        REQUIRE_EQ(new_rc.ref_count_weak(), 1);

        RCWeak<TestRCBase> new_weak = new_rc;
        new_weak.swap(weak);
        REQUIRE_EQ(new_weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(new_rc.ref_count_weak(), 2);
        REQUIRE_EQ(rc.ref_count_weak(), 2);
        REQUIRE_EQ(new_rc.get(), weak.get_unsafe());
        REQUIRE_EQ(rc.get(), new_weak.get_unsafe());
    }

    // [need't test] skr hash
    SUBCASE("test empty")
    {
        RCWeak<TestRCBase> empty{};
        RCWeak<TestRCBase> empty_copy{ empty };
        RCWeak<TestRCBase> empty_move{ std::move(empty) };
        RCWeak<TestRCBase> empty_assign, empty_move_assign;
        empty_assign      = empty;
        empty_move_assign = std::move(empty);

        empty.ref_count_weak();
        empty.is_empty();
        empty.is_alive();
        empty.is_expired();
        empty.operator bool();
        empty.reset();
        empty.reset(nullptr);
        empty.swap(empty_copy);
        empty.lock();
    }
}

TEST_CASE("Test Custom Deleter")
{
    using namespace skr;

    RC<TestCustomDeleterRC> rc{ SkrNew<TestCustomDeleterRC>() };
    rc.reset();
    REQUIRE_EQ(rc_custom_deleter_count, 1);

    RCUnique<TestCustomDeleterRC> rc_unique{ SkrNew<TestCustomDeleterRC>() };
    rc_unique.reset();
    REQUIRE_EQ(rc_custom_deleter_count, 2);

    {
        RC<TestCustomDeleterRC> rc{ SkrNew<TestCustomDeleterRC>() };
    }
    REQUIRE_EQ(rc_custom_deleter_count, 3);

    {
        RCUnique<TestCustomDeleterRC> rc_unique{ SkrNew<TestCustomDeleterRC>() };
    }
    REQUIRE_EQ(rc_custom_deleter_count, 4);
}