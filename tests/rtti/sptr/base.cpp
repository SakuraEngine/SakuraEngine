#include "test.hpp"

class SPTRCommon : public SPTRBase
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST(SPTRCommon, Base)
{
    {
        skr::SPtr<int> pT1 = {};
        EXPECT_EQ(pT1, nullptr);
        EXPECT_EQ(pT1.get(), nullptr);
    }
    {
        skr::SPtr<int> pT1(SkrNew<int>(5));
        EXPECT_EQ(*pT1, 5);
        EXPECT_EQ(pT1.use_count(), 1);
        EXPECT_TRUE(pT1.unique());

        skr::SPtr<int> pT2 = {};
        EXPECT_NE(pT1, pT2);
        EXPECT_EQ(pT2.use_count(), 0);
        EXPECT_TRUE(!pT2.unique());

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
		EXPECT_TRUE(!pT2.unique());
		EXPECT_EQ(pT2.use_count(), 0);
		EXPECT_TRUE(pT1.unique());
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
        { --mCount; }
};
int A::mCount = 0;

struct B : public A
{
    B() = default;
    template<typename...Args> B(Args&&... c) : A(std::forward<Args>(c)...) {}
};

#include <memory>

TEST(SPTRCommon, Ctors)
{
    EXPECT_TRUE(A::mCount == 0);

    skr::SPtr<A> pT2(SkrNew<A>(0));
    EXPECT_TRUE(A::mCount == 1);
    EXPECT_TRUE(pT2->mc == 0);
    EXPECT_TRUE(pT2.use_count() == 1);
    EXPECT_TRUE(pT2.unique());

    pT2.reset(SkrNew<A>(1));
    EXPECT_TRUE(pT2->mc == 1);
    EXPECT_TRUE(A::mCount == 1);
    EXPECT_TRUE(pT2.use_count() == 1);
    EXPECT_TRUE(pT2.unique());

    skr::SPtr<A> pT3(SkrNew<A>(2));
    EXPECT_TRUE(A::mCount == 2);

    pT2.swap(pT3);
    EXPECT_TRUE(pT2->mc == 2);
    EXPECT_TRUE(pT3->mc == 1);
    EXPECT_TRUE(A::mCount == 2);

    pT2.swap(pT3);
    EXPECT_TRUE(pT2->mc == 1);
    EXPECT_TRUE(pT3->mc == 2);
    EXPECT_TRUE(A::mCount == 2);
    if(!pT2)
        EXPECT_TRUE(!pT2.get()); // Will fail

    skr::SPtr<A> pT4;
    EXPECT_TRUE(pT2.use_count() == 1);
    EXPECT_TRUE(pT2.unique());
    EXPECT_TRUE(A::mCount == 2);
    if(pT4)
        EXPECT_TRUE(pT4.get()); // Will fail
    if(!(!pT4))
        EXPECT_TRUE(pT4.get()); // Will fail

    pT4 = pT2;
    EXPECT_TRUE(pT2.use_count() == 2);
    EXPECT_TRUE(pT4.use_count() == 2);
    EXPECT_TRUE(!pT2.unique());
    EXPECT_TRUE(!pT4.unique());
    EXPECT_TRUE(A::mCount == 2);
    EXPECT_TRUE(pT2 == pT4);
    EXPECT_TRUE(pT2 != pT3);
    EXPECT_TRUE(!(pT2 < pT4)); // They should be equal
    EXPECT_TRUE(!(pT2 > pT4)); // They should be equal

    skr::SPtr<A> pT5(pT4);
    EXPECT_TRUE(pT4 == pT5);
    EXPECT_TRUE(pT2.use_count() == 3);
    EXPECT_TRUE(pT4.use_count() == 3);
    EXPECT_TRUE(pT5.use_count() == 3);
    EXPECT_TRUE(!pT5.unique());

    // STL
    std::shared_ptr<A> pTT4 = std::shared_ptr<A>(nullptr);
    EXPECT_NE(pTT4.use_count(), 1);
    EXPECT_EQ(pTT4.use_count(), 0);

    pTT4 = std::shared_ptr<A>((A*)nullptr);
    EXPECT_EQ(pTT4.use_count(), 1); // 1.1
    EXPECT_EQ(pTT4.use_count(), 1); // 1.2

    // SPtr
    pT4 = skr::SPtr<A>(nullptr);
    EXPECT_TRUE(!pT4.unique());
    EXPECT_EQ(pT4.use_count(), 0);

    pT4 = skr::SPtr<A>((A*)nullptr);
    EXPECT_TRUE(!pT4.unique()); // 2.1
    EXPECT_EQ(pT4.use_count(), 0); // 2.2

    // CAUTION: Look at 1.1/2 & 2.1/2
    // We do not support the same behavior as std::shared_ptr to initialize pRC with [T* = null] because it is stupid.

    EXPECT_EQ(pT2.use_count(), 2);

    EXPECT_TRUE(A::mCount == 2);
}

TEST(SPTRCommon, Move)
{
    {
        skr::SPtr<A> rT1(SkrNew<A>(42));
        skr::SPtr<B> rT2(SkrNew<B>());  // default ctor uses 0
        rT2->mc = 115;

        EXPECT_TRUE(rT1->mc == 42);
        EXPECT_TRUE(rT2->mc == 115);

        rT1 = std::move(rT2);

        EXPECT_TRUE(rT1->mc == 115);
        // EATEST_VERIFY(rT2->mc == 115);  // state of object post-move is undefined.
    }
}

TEST(SPTRCommon, StdWeak)
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

TEST(SPTRCommon, Weak)
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
