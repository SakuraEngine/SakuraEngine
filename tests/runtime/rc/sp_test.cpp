#include "SkrTestFramework/framework.hpp"

#include <SkrCore/memory/sp.hpp>

static uint64_t sp_base_count           = 0;
static uint64_t sp_derived_count        = 0;
static uint64_t sp_custom_deleter_count = 0;

struct TestSPBase {
    TestSPBase()
    {
        ++sp_base_count;
    }
    virtual ~TestSPBase()
    {
        --sp_base_count;
    }
};

struct TestSPDerived : public TestSPBase {
public:
    TestSPDerived()
    {
        ++sp_derived_count;
    }
    ~TestSPDerived()
    {
        --sp_derived_count;
    }
};

struct TestCustomDeleterSP {
    inline void skr_sp_delete()
    {
        ++sp_custom_deleter_count;
        SkrDelete(this);
    }
};

TEST_CASE("Test UPtr")
{
    using namespace skr;
    SUBCASE("ctor & dtor")
    {
        UPtr<TestSPBase> empty_ctor{};
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(empty_ctor.get(), nullptr);
        REQUIRE(empty_ctor.is_empty());

        UPtr<TestSPBase> null_ctor{ nullptr };
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(null_ctor.get(), nullptr);
        REQUIRE(null_ctor.is_empty());

        {
            UPtr<TestSPBase> pointer_ctor{ SkrNew<TestSPBase>() };
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_NE(pointer_ctor.get(), nullptr);
            REQUIRE_FALSE(pointer_ctor.is_empty());
        }

        {
            UPtr<TestSPBase> pointer_ctor{ SkrNew<TestSPDerived>() };
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_NE(pointer_ctor.get(), nullptr);
            REQUIRE_FALSE(pointer_ctor.is_empty());
        }

        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    SUBCASE("copy & move")
    {
        UPtr<TestSPBase>    base{ SkrNew<TestSPBase>() };
        UPtr<TestSPDerived> derived{ SkrNew<TestSPDerived>() };

        auto             cached_base = base.get();
        UPtr<TestSPBase> move_base   = std::move(base);
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base.get(), nullptr);
        REQUIRE_EQ(move_base.get(), cached_base);

        auto             cached_derived = derived.get();
        UPtr<TestSPBase> move_derived   = std::move(derived);
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(derived.get(), nullptr);
        REQUIRE_EQ(move_derived.get(), cached_derived);

        // will not pass compile
        // UPtr<TestSPBase> copy_base    = move_base;
        // UPtr<TestSPBase> copy_derived = move_derived;

        move_base    = nullptr;
        move_derived = nullptr;
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    SUBCASE("assign & move assign")
    {
        UPtr<TestSPBase>    base{ SkrNew<TestSPBase>() };
        UPtr<TestSPDerived> derived{ SkrNew<TestSPDerived>() };

        auto             cached_base = base.get();
        UPtr<TestSPBase> move_assign_base{};
        move_assign_base = std::move(base);
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base.get(), nullptr);
        REQUIRE_EQ(move_assign_base.get(), cached_base);

        auto             cached_derived = derived.get();
        UPtr<TestSPBase> move_assign_derived{};
        move_assign_derived = std::move(derived);
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(derived.get(), nullptr);
        REQUIRE_EQ(move_assign_derived.get(), cached_derived);

        // will not pass compile
        // UPtr<TestSPBase> assign_base{};
        // assign_base = move_base;
        // UPtr<TestSPBase> assign_derived {}
        // assign_derived = move_derived;

        move_assign_base    = nullptr;
        move_assign_derived = nullptr;
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    // [need't test] factory
    // [need't test] getter

    SUBCASE("test is empty")
    {
        UPtr<TestSPBase> empty{};
        UPtr<TestSPBase> something{ SkrNew<TestSPBase>() };

        REQUIRE_EQ(empty.is_empty(), true);
        REQUIRE_EQ(something.is_empty(), false);
        REQUIRE_EQ(empty, false);
        REQUIRE_EQ(something, true);
    }

    SUBCASE("test ops")
    {
        UPtr<TestSPBase> sp = SkrNew<TestSPBase>();
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(sp.get(), nullptr);

        sp.reset();
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(sp.get(), nullptr);

        sp.reset(SkrNew<TestSPBase>());
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(sp.get(), nullptr);

        UPtr<TestSPBase> other;
        sp.swap(other);
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(sp.get(), nullptr);
        REQUIRE_NE(other.get(), nullptr);

        other.reset(SkrNew<TestSPBase>());
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(other.get(), nullptr);
    }

    // [need't test] pointer behaviour
    // [need't test] skr hash

    SUBCASE("test empty")
    {
        UPtr<TestSPBase> empty{};
        UPtr<TestSPBase> empty_move{ std::move(empty) };
        UPtr<TestSPBase> empty_move_assign;
        empty_move_assign = std::move(empty);

        empty.get();
        empty.is_empty();
        empty.operator bool();
        empty.reset();
        empty.reset(nullptr);
        empty.release();
        empty.swap(empty_move);
    }
}

TEST_CASE("Test SP")
{
    using namespace skr;

    SUBCASE("ctor & dtor")
    {
        SP<TestSPBase> empty_ctor{};
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(empty_ctor.get(), nullptr);
        REQUIRE_EQ(empty_ctor.ref_count(), 0);
        REQUIRE_EQ(empty_ctor.ref_count_weak(), 0);

        SP<TestSPBase> null_ctor{ nullptr };
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(null_ctor.get(), nullptr);
        REQUIRE_EQ(null_ctor.ref_count(), 0);
        REQUIRE_EQ(null_ctor.ref_count_weak(), 0);

        {
            SP<TestSPBase> pointer_ctor{ SkrNew<TestSPBase>() };
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_NE(pointer_ctor.get(), nullptr);
            REQUIRE_EQ(pointer_ctor.ref_count(), 1);
            REQUIRE_EQ(pointer_ctor.ref_count_weak(), 1);
        }

        {
            UPtr<TestSPBase> unique{ SkrNew<TestSPBase>() };
            SP<TestSPBase>   sp{ std::move(unique) };
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_NE(sp.get(), nullptr);
            REQUIRE_EQ(sp.ref_count(), 1);
            REQUIRE_EQ(sp.ref_count_weak(), 1);

            REQUIRE_EQ(unique.get(), nullptr);
        }

        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    SUBCASE("copy & move")
    {
        SP<TestSPBase>    base{ SkrNew<TestSPBase>() };
        SP<TestSPDerived> derived{ SkrNew<TestSPDerived>() };

        SP<TestSPBase> copy_base{ base };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base.get(), copy_base.get());
        REQUIRE_EQ(base.ref_count(), 2);
        REQUIRE_EQ(copy_base.ref_count(), 2);
        REQUIRE_EQ(base.ref_count_weak(), 2);
        REQUIRE_EQ(copy_base.ref_count_weak(), 2);

        SP<TestSPBase> copy_derived{ derived };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(derived.get(), copy_derived.get());
        REQUIRE_EQ(derived.ref_count(), 2);
        REQUIRE_EQ(copy_derived.ref_count(), 2);
        REQUIRE_EQ(derived.ref_count_weak(), 2);
        REQUIRE_EQ(copy_derived.ref_count_weak(), 2);

        SP<TestSPBase> move_base{ std::move(base) };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base.get(), nullptr);
        REQUIRE_EQ(move_base.get(), copy_base.get());
        REQUIRE_EQ(base.ref_count(), 0);
        REQUIRE_EQ(copy_base.ref_count(), 2);
        REQUIRE_EQ(move_base.ref_count(), 2);
        REQUIRE_EQ(base.ref_count_weak(), 0);
        REQUIRE_EQ(copy_base.ref_count_weak(), 2);

        SP<TestSPBase> move_derived{ std::move(derived) };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(derived.get(), nullptr);
        REQUIRE_EQ(move_derived.get(), copy_derived.get());
        REQUIRE_EQ(derived.ref_count(), 0);
        REQUIRE_EQ(copy_derived.ref_count(), 2);
        REQUIRE_EQ(move_derived.ref_count(), 2);
        REQUIRE_EQ(derived.ref_count_weak(), 0);
        REQUIRE_EQ(copy_derived.ref_count_weak(), 2);

        copy_base    = nullptr;
        move_base    = nullptr;
        copy_derived = nullptr;
        move_derived = nullptr;

        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    SUBCASE("assign & move assign")
    {
        SP<TestSPBase>    base{ SkrNew<TestSPBase>() };
        SP<TestSPDerived> derived{ SkrNew<TestSPDerived>() };

        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base.ref_count(), 1);
        REQUIRE_EQ(derived.ref_count(), 1);
        {
            // assign
            SP<TestSPBase> assign{};
            assign = base;
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 2);
            REQUIRE_EQ(derived.ref_count(), 1);
            REQUIRE_EQ(assign.ref_count(), 2);
            REQUIRE_EQ(assign.get(), base.get());

            // assign derived
            assign = derived;
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 1);
            REQUIRE_EQ(derived.ref_count(), 2);
            REQUIRE_EQ(assign.ref_count(), 2);
            REQUIRE_EQ(assign.get(), derived.get());
        }

        {
            // move assign
            SP<TestSPBase> move_assign{};
            auto           cached_base = base.get();
            move_assign                = std::move(base);
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 0);
            REQUIRE_EQ(derived.ref_count(), 1);
            REQUIRE_EQ(move_assign.ref_count(), 1);
            REQUIRE_EQ(move_assign.get(), cached_base);
            REQUIRE_EQ(base.get(), nullptr);

            // move assign derived
            auto cached_derived = derived.get();
            move_assign         = std::move(derived);
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(base.ref_count(), 0);
            REQUIRE_EQ(derived.ref_count(), 0);
            REQUIRE_EQ(move_assign.ref_count(), 1);
            REQUIRE_EQ(move_assign.get(), cached_derived);
            REQUIRE_EQ(derived.get(), nullptr);
        }

        {
            UPtr<TestSPDerived> unique{ SkrNew<TestSPDerived>() };
            SP<TestSPBase>      move_assign_unique{};
            auto                cached_unique = unique.get();
            move_assign_unique                = std::move(unique);
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 1);
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
        SP<TestSPBase>    base1{ SkrNew<TestSPBase>() };
        SP<TestSPBase>    base2{ SkrNew<TestSPBase>() };
        SP<TestSPDerived> derived1{ SkrNew<TestSPDerived>() };
        SP<TestSPDerived> derived2{ SkrNew<TestSPDerived>() };

        SP<TestSPBase> copy_base1    = base1;
        SP<TestSPBase> copy_derived1 = derived1;

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
        SP<TestSPBase> empty{};
        SP<TestSPBase> something{ SkrNew<TestSPBase>() };

        REQUIRE_EQ(empty.is_empty(), true);
        REQUIRE_EQ(something.is_empty(), false);
        REQUIRE_EQ(empty, false);
        REQUIRE_EQ(something, true);
    }

    SUBCASE("test ops")
    {
        SP<TestSPBase> sp = SkrNew<TestSPBase>();
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(sp.get(), nullptr);

        sp.reset();
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(sp.get(), nullptr);

        sp.reset(SkrNew<TestSPBase>());
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(sp.get(), nullptr);

        SP<TestSPBase> other;
        sp.swap(other);
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_EQ(sp.get(), nullptr);
        REQUIRE_NE(other.get(), nullptr);

        other.reset(SkrNew<TestSPBase>());
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(other.get(), nullptr);
    }

    // [need't test] pointer behaviour
    // [need't test] skr hash

    SUBCASE("test empty")
    {
        SP<TestSPBase> empty{};
        SP<TestSPBase> empty_copy{ empty };
        SP<TestSPBase> empty_move{ std::move(empty) };
        SP<TestSPBase> empty_assign, empty_move_assign;
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

TEST_CASE("Test SPWeak")
{
    using namespace skr;

    SUBCASE("test ctor")
    {
        SPWeak<TestSPBase> empty_ctor{};
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE(empty_ctor.lock().is_empty());
        REQUIRE_EQ(empty_ctor.ref_count_weak(), 0);

        SPWeak<TestSPBase> null_ctor{ nullptr };
        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE(null_ctor.lock().is_empty());
        REQUIRE_EQ(null_ctor.ref_count_weak(), 0);

        // from sp
        {
            auto sp      = SP<TestSPBase>::New();
            auto sp_weak = SPWeak<TestSPBase>(sp);
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_FALSE(sp_weak.lock().is_empty());
            REQUIRE_EQ(sp_weak.ref_count_weak(), 2);
            REQUIRE_EQ(sp.ref_count_weak(), 2);
            REQUIRE_EQ(sp.ref_count(), 1);
            if (auto locked = sp_weak.lock())
            {
                REQUIRE_EQ(sp_base_count, 1);
                REQUIRE_EQ(sp_derived_count, 0);
                REQUIRE_EQ(locked.get(), sp.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        // from sp derived
        {
            auto sp      = SP<TestSPDerived>::New();
            auto sp_weak = SPWeak<TestSPBase>(sp);
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_FALSE(sp_weak.lock().is_empty());
            REQUIRE_EQ(sp_weak.ref_count_weak(), 2);
            REQUIRE_EQ(sp.ref_count_weak(), 2);
            REQUIRE_EQ(sp.ref_count(), 1);
            if (auto locked = sp_weak.lock())
            {
                REQUIRE_EQ(sp_base_count, 1);
                REQUIRE_EQ(sp_derived_count, 1);
                REQUIRE_EQ(locked.get(), sp.get());
            }
            else
            {
                SKR_UNREACHABLE_CODE()
            }
        }

        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    SUBCASE("copy & move")
    {
        auto base    = SP<TestSPBase>::New();
        auto derived = SP<TestSPDerived>::New();

        SPWeak<TestSPBase>    base_weak{ base };
        SPWeak<TestSPDerived> derived_weak{ derived };

        SPWeak<TestSPBase> copy_base{ base_weak };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base_weak.ref_count_weak(), 3);
        REQUIRE_EQ(copy_base.ref_count_weak(), 3);
        REQUIRE_EQ(copy_base.lock().get(), base_weak.lock().get());

        SPWeak<TestSPBase> copy_derived{ derived_weak };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(derived_weak.ref_count_weak(), 3);
        REQUIRE_EQ(copy_derived.ref_count_weak(), 3);
        REQUIRE_EQ(copy_derived.lock().get(), derived_weak.lock().get());

        SPWeak<TestSPBase> move_base{ std::move(base_weak) };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
        REQUIRE_EQ(base.ref_count_weak(), 3);
        REQUIRE_EQ(move_base.ref_count_weak(), 3);
        REQUIRE_EQ(move_base.lock().get(), base.get());

        SPWeak<TestSPBase> move_derived{ std::move(derived_weak) };
        REQUIRE_EQ(sp_base_count, 2);
        REQUIRE_EQ(sp_derived_count, 1);
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

        REQUIRE_EQ(sp_base_count, 0);
        REQUIRE_EQ(sp_derived_count, 0);
    }

    SUBCASE("assign & move assign")
    {
        auto base    = SP<TestSPBase>::New();
        auto derived = SP<TestSPDerived>::New();

        SPWeak<TestSPBase>    base_weak{ base };
        SPWeak<TestSPDerived> derived_weak{ derived };

        // assign weak
        {
            SPWeak<TestSPBase> assign{};

            // assign
            assign = base_weak;
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(base_weak.ref_count_weak(), 3);
            REQUIRE_EQ(assign.ref_count_weak(), 3);
            REQUIRE_EQ(assign.lock().get(), base_weak.lock().get());

            // assign derived
            assign = derived_weak;
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(derived_weak.ref_count_weak(), 3);
            REQUIRE_EQ(assign.ref_count_weak(), 3);
            REQUIRE_EQ(assign.lock().get(), derived_weak.lock().get());

            REQUIRE_EQ(base_weak.ref_count_weak(), 2);
        }

        // move assign weak
        {
            SPWeak<TestSPBase> move_assign{};

            // move assign base
            auto cached_base = base_weak.lock().get();
            move_assign      = std::move(base_weak);
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE(base_weak.is_empty());
            REQUIRE_EQ(move_assign.ref_count_weak(), 2);
            REQUIRE_EQ(move_assign.lock().get(), cached_base);

            // move assign derived
            auto cached_derived = derived_weak.lock().get();
            move_assign         = std::move(derived_weak);
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE(derived_weak.is_empty());
            REQUIRE_EQ(move_assign.ref_count_weak(), 2);
            REQUIRE_EQ(move_assign.lock().get(), cached_derived);

            REQUIRE_EQ(base_weak.ref_count_weak(), 0);
        }

        // assign sp & unique sp
        {
            SPWeak<TestSPBase> assign{};
            assign = base;
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(base.ref_count_weak(), 2);
            REQUIRE_EQ(assign.ref_count_weak(), 2);
            REQUIRE_EQ(assign.lock().get(), base.get());

            // assign derived
            assign = derived;
            REQUIRE_EQ(sp_base_count, 2);
            REQUIRE_EQ(sp_derived_count, 1);
            REQUIRE_EQ(derived.ref_count_weak(), 2);
            REQUIRE_EQ(assign.ref_count_weak(), 2);
            REQUIRE_EQ(assign.lock().get(), derived.get());
        }
    }

    SUBCASE("test compare")
    {
        SP<TestSPBase>    sp_base1{ SkrNew<TestSPBase>() };
        SP<TestSPBase>    sp_base2{ SkrNew<TestSPBase>() };
        SP<TestSPDerived> sp_derived1{ SkrNew<TestSPDerived>() };
        SP<TestSPDerived> sp_derived2{ SkrNew<TestSPDerived>() };

        SPWeak<TestSPBase>    base1{ sp_base1 };
        SPWeak<TestSPBase>    base2{ sp_base2 };
        SPWeak<TestSPDerived> derived1{ sp_derived1 };
        SPWeak<TestSPDerived> derived2{ sp_derived2 };

        SPWeak<TestSPBase> copy_base1    = base1;
        SPWeak<TestSPBase> copy_derived1 = derived1;

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
        SP<TestSPBase> sp{ SkrNew<TestSPBase>() };

        // basic lock
        SPWeak<TestSPBase> weak{ sp };
        REQUIRE(weak.is_alive());
        REQUIRE_FALSE(weak.is_expired());
        REQUIRE_FALSE(weak.lock().is_empty());
        {
            auto locked = weak.lock();
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_EQ(locked.get(), sp.get());
        }

        // lock a destroyed weak
        sp.reset();
        REQUIRE_FALSE(weak.is_alive());
        REQUIRE(weak.is_expired());
        REQUIRE(weak.lock().is_empty());
        {
            auto locked = weak.lock();
            REQUIRE_EQ(sp_base_count, 0);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_EQ(locked.get(), nullptr);
        }

        // lock
        sp   = SkrNew<TestSPBase>();
        weak = sp;
        REQUIRE(weak.is_alive());
        REQUIRE_FALSE(weak.is_expired());
        REQUIRE_FALSE(weak.lock().is_empty());
        {
            SP<TestSPBase> locked_sp = weak.lock();
            REQUIRE_EQ(sp_base_count, 1);
            REQUIRE_EQ(sp_derived_count, 0);
            REQUIRE_EQ(locked_sp.get(), sp.get());
            REQUIRE_EQ(locked_sp.ref_count(), 2);
        }
    }

    // [need't test] is empty

    SUBCASE("test ops")
    {
        SP<TestSPBase> sp = SkrNew<TestSPBase>();
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_NE(sp.get(), nullptr);

        SPWeak<TestSPBase> weak{ sp };
        REQUIRE_EQ(sp_base_count, 1);
        REQUIRE_EQ(sp_derived_count, 0);
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), sp.get());

        weak.reset();
        REQUIRE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 0);
        REQUIRE_EQ(weak.get_unsafe(), nullptr);
        REQUIRE_EQ(sp.ref_count_weak(), 1);

        weak.reset(sp);
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), sp.get());
        REQUIRE_EQ(sp.ref_count_weak(), 2);

        SP<TestSPBase> new_sp = SkrNew<TestSPBase>();
        weak.reset(new_sp);
        REQUIRE_FALSE(weak.is_empty());
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.get_unsafe(), new_sp.get());
        REQUIRE_EQ(sp.ref_count_weak(), 1);
        REQUIRE_EQ(new_sp.ref_count_weak(), 2);

        SPWeak<TestSPBase> new_weak = sp;
        new_weak.swap(weak);
        REQUIRE_EQ(new_weak.ref_count_weak(), 2);
        REQUIRE_EQ(weak.ref_count_weak(), 2);
        REQUIRE_EQ(new_sp.ref_count_weak(), 2);
        REQUIRE_EQ(sp.ref_count_weak(), 2);
        REQUIRE_EQ(new_sp.get(), new_weak.get_unsafe());
        REQUIRE_EQ(sp.get(), weak.get_unsafe());
    }

    // [need't test] skr hash

    SUBCASE("test empty")
    {
        SPWeak<TestSPBase> empty{};
        SPWeak<TestSPBase> empty_copy{ empty };
        SPWeak<TestSPBase> empty_move{ std::move(empty) };
        SPWeak<TestSPBase> empty_assign, empty_move_assign;
        empty_assign      = empty;
        empty_move_assign = std::move(empty);

        empty.ref_count_weak();
        empty.is_empty();
        empty.is_alive();
        empty.is_expired();
        empty.operator bool();
        empty.reset();
        empty.swap(empty_copy);
        empty.lock();
    }
}

TEST_CASE("Test Custom Deleter")
{
    using namespace skr;

    UPtr<TestCustomDeleterSP> uptr{ SkrNew<TestCustomDeleterSP>() };
    uptr.reset();
    REQUIRE_EQ(sp_custom_deleter_count, 1);

    {
        UPtr<TestCustomDeleterSP> uptr{ SkrNew<TestCustomDeleterSP>() };
        REQUIRE_EQ(sp_custom_deleter_count, 1);
    }
    REQUIRE_EQ(sp_custom_deleter_count, 2);
}