#include "test.hpp"

struct SPTRCommonTests
{

};

TEST_CASE_METHOD(SPTRCommonTests, "Base")
{
    {
        skr::SPtr<int> pT1 = {};
        EXPECT_EQ(pT1, nullptr);
        EXPECT_EQ(pT1.get(), nullptr);
    }
    {
        auto ptr = SkrNew<int>(5);
        EXPECT_NE(ptr, nullptr);
        skr::SPtr<int> pT1(ptr);
        EXPECT_EQ(*pT1, 5);
    }
    {
        skr::SPtr<int> pT1(SkrNew<int>(5));
        EXPECT_EQ(*pT1, 5);
        EXPECT_EQ(pT1.use_count(), 1);
        REQUIRE(pT1.unique());

        skr::SPtr<int> pT2 = {};
        EXPECT_NE(pT1, pT2);
        EXPECT_EQ(pT2.use_count(), 0);
        REQUIRE(!pT2.unique());

        pT2 = pT1;
		EXPECT_EQ(pT1.use_count(), 2);
		EXPECT_EQ(pT2.use_count(), 2);
		EXPECT_FALSE(pT1.unique());
		EXPECT_FALSE(pT1 < pT2); // They should be equal
		EXPECT_EQ(pT1, pT2);

        *pT1 = 3;
		EXPECT_EQ(*pT1, 3);
		EXPECT_EQ(*pT1, 3);
		EXPECT_EQ(*pT2, 3);

        pT2.reset();
		REQUIRE(!pT2.unique());
		EXPECT_EQ(pT2.use_count(), 0);
		REQUIRE(pT1.unique());
		EXPECT_EQ(pT1.use_count(), 1);
		EXPECT_NE(pT1, pT2);
    }
}

struct A
{
    char mc;
    static int mCount;

    A(int c) 
        : mc(c) { ++mCount; }
    
    A(char c = 0) 
        : mc(c) { ++mCount; }

    A(const A& x) 
        : mc(x.mc) { ++mCount; }

    A& operator=(const A& x) 
        { mc = x.mc; return *this; }

    virtual ~A() // Virtual because we subclass A below.
    { 
        --mCount;
    }
};
int A::mCount = 0;

struct B : public A
{
    B() = default;
    template<typename...Args> B(Args&&... c) : A(std::forward<Args>(c)...) {}
};

#include <memory>

TEST_CASE_METHOD(SPTRCommonTests, "Ctors")
{
    REQUIRE(A::mCount == 0);
    {
        skr::SPtr<A> pT2(SkrNew<A>(0));
        REQUIRE(A::mCount == 1);
        REQUIRE(pT2->mc == 0);
        REQUIRE(pT2.use_count() == 1);
        REQUIRE(pT2.unique());

        pT2.reset(SkrNew<A>(1));
        REQUIRE(pT2->mc == 1);
        REQUIRE(A::mCount == 1);
        REQUIRE(pT2.use_count() == 1);
        REQUIRE(pT2.unique());

        skr::SPtr<A> pT3(SkrNew<A>(2));
        REQUIRE(A::mCount == 2);

        pT2.swap(pT3);
        REQUIRE(pT2->mc == 2);
        REQUIRE(pT3->mc == 1);
        REQUIRE(A::mCount == 2);

        pT2.swap(pT3);
        REQUIRE(pT2->mc == 1);
        REQUIRE(pT3->mc == 2);
        REQUIRE(A::mCount == 2);
        if(!pT2)
            REQUIRE(!pT2.get()); // Will fail

        skr::SPtr<A> pT4;
        REQUIRE(pT2.use_count() == 1);
        REQUIRE(pT2.unique());
        REQUIRE(A::mCount == 2);
        if(pT4)
            REQUIRE(pT4.get()); // Will fail
        if(!(!pT4))
            REQUIRE(pT4.get()); // Will fail

        pT4 = pT2;
        REQUIRE(pT2.use_count() == 2);
        REQUIRE(pT4.use_count() == 2);
        REQUIRE(!pT2.unique());
        REQUIRE(!pT4.unique());
        REQUIRE(A::mCount == 2);
        REQUIRE(pT2 == pT4);
        REQUIRE(pT2 != pT3);
        REQUIRE(!(pT2 < pT4)); // They should be equal
        REQUIRE(!(pT2 > pT4)); // They should be equal

        skr::SPtr<A> pT5(pT4);
        REQUIRE(pT4 == pT5);
        REQUIRE(pT2.use_count() == 3);
        REQUIRE(pT4.use_count() == 3);
        REQUIRE(pT5.use_count() == 3);
        REQUIRE(!pT5.unique());

        // STL
        std::shared_ptr<A> pTT4 = std::shared_ptr<A>(nullptr);
        EXPECT_NE(pTT4.use_count(), 1);
        EXPECT_EQ(pTT4.use_count(), 0);

        pTT4 = std::shared_ptr<A>((A*)nullptr);
        EXPECT_EQ(pTT4.use_count(), 1); // 1.1
        EXPECT_EQ(pTT4.use_count(), 1); // 1.2

        // SPtr
        pT4 = skr::SPtr<A>(nullptr);
        REQUIRE(!pT4.unique());
        EXPECT_EQ(pT4.use_count(), 0);

        pT4 = skr::SPtr<A>((A*)nullptr);
        REQUIRE(!pT4.unique()); // 2.1
        EXPECT_EQ(pT4.use_count(), 0); // 2.2

        // CAUTION: Look at 1.1/2 & 2.1/2
        // We do not support the same behavior as std::shared_ptr to initialize pRC with [T* = null] because it is stupid.

        EXPECT_EQ(pT2.use_count(), 2);

        REQUIRE(A::mCount == 2);
    }
    REQUIRE(A::mCount == 0);
}

TEST_CASE_METHOD(SPTRCommonTests, "Move")
{
    skr::SPtr<A> rT1(SkrNew<A>(42));
    skr::SPtr<B> rT2(SkrNew<B>());  // default ctor uses 0
    rT2->mc = 115;

    REQUIRE(rT1->mc == 42);
    REQUIRE(rT2->mc == 115);

    rT1 = std::move(rT2);

    REQUIRE(rT1->mc == 115);
    // EATEST_VERIFY(rT2->mc == 115);  // state of object post-move is undefined.
}

TEST_CASE_METHOD(SPTRCommonTests, "StdWeak")
{
    std::shared_ptr<B> pC(new B (88));
    std::weak_ptr<B> wpC(pC);
    std::weak_ptr<B> wpC2 = {};
    wpC2 = pC;

    EXPECT_EQ(wpC.use_count(), 1);

    auto pC2 = wpC2.lock();
    EXPECT_EQ(pC, pC2);
    EXPECT_EQ(pC.get(), pC2.get());
    EXPECT_EQ(pC2.use_count(), 2);
}

TEST_CASE_METHOD(SPTRCommonTests, "Weak")
{
    skr::SPtr<B> pC(SkrNew<B>(88));
    skr::SWeakPtr<B> wpC(pC);
    skr::SWeakPtr<B> wpC2 = {};
    EXPECT_EQ(wpC2.use_count(), 0);
    wpC2 = pC;

    EXPECT_EQ(wpC.use_count(), 1);
    EXPECT_EQ(wpC2.use_count(), 1);

    auto pC2 = wpC2.lock();
    EXPECT_EQ(pC.get(), pC2.get());
    EXPECT_EQ(pC, pC2);
    EXPECT_EQ(pC2.use_count(), 2);
}