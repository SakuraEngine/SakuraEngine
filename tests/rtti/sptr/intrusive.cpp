#include "test.hpp"

struct TestObject : public skr::SInterface
{
    enum Status
    {
        Uninitialized,
        Created,
        Hosted,
        Destroyed
    };
    TestObject(Status& code)
        : code(code)
    {
        code = Status::Created;
    }
    ~TestObject()
    {
        code = Status::Destroyed;
    }
    uint32_t add_refcount()
    {
        code = Status::Hosted;
        rc++;
        return rc;
    }
    uint32_t release()
    {
        rc--;
        return rc;
    }
    uint32_t use_count() const
    {
        return rc;
    }
    Status& code;
    std::atomic_uint32_t rc = 0;
};

struct TestSon : public TestObject
{
    TestSon(Status& code) : TestObject(code) { }
};

TEST(SPTR, CopyIntrusive)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    {
        auto mTestObject = skr::SPtr<TestObject>::Create(status);
        EXPECT_EQ(mTestObject->rc, 1);
        skr::SPtr<TestObject> mCopy(mTestObject);
        EXPECT_EQ(mTestObject->rc, 2);
        auto mCopy1 = mTestObject;
        EXPECT_EQ(mTestObject->rc, 3);
    }
    EXPECT_EQ(status, TestObject::Status::Destroyed);
}

TEST(SPTR, CopyIntrusive2)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    {
        auto mTestObject = skr::SPtr<TestObject>::Create(status);
        EXPECT_EQ(mTestObject->rc, 1);
        skr::SPtr<TestObject> mCopy(mTestObject);
        EXPECT_EQ(mTestObject->rc, 2);
        auto mCopy1 = mTestObject;
        EXPECT_EQ(mTestObject->rc, 3);
    }
    EXPECT_EQ(status, TestObject::Status::Destroyed);
}

TEST(SPTR, SwapIntrusive)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    auto mOneObject = skr::SPtr<TestObject>::Create(one_status);
    {
        auto mAnotherObject = skr::SPtr<TestObject>::Create(another_status);
        mOneObject = mAnotherObject;     
        EXPECT_EQ(one_status, TestObject::Status::Destroyed);
    }
    EXPECT_EQ(another_status, TestObject::Status::Hosted);
}

TEST(SPTR, SwapIntrusive2)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    auto mOneObject = skr::SPtr<TestObject>::Create(one_status);
    {
        auto mAnotherObject = skr::SPtr<TestObject>::Create(another_status);
        mOneObject = mAnotherObject;     
        EXPECT_EQ(one_status, TestObject::Status::Destroyed);
    }
    EXPECT_EQ(another_status, TestObject::Status::Hosted);
}

TEST(SPTR, MoveIntrusive)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    skr::SPtr<TestObject> rT1(SkrNew<TestObject>(one_status));
    skr::SPtr<TestSon> rT2(SkrNew<TestSon>(another_status));  // default ctor uses 0

    EXPECT_EQ(one_status, TestObject::Status::Hosted);
    EXPECT_EQ(another_status, TestObject::Status::Hosted);

    rT1 = std::move(rT2);

    EXPECT_EQ(one_status, TestObject::Status::Destroyed);
}

TEST(SPTR, MoveIntrusive2)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    skr::SPtr<TestObject> rT1(SkrNew<TestObject>(one_status));
    skr::SPtr<TestSon> rT2(SkrNew<TestSon>(another_status));  // default ctor uses 0

    EXPECT_EQ(one_status, TestObject::Status::Hosted);
    EXPECT_EQ(another_status, TestObject::Status::Hosted);

    rT1 = std::move(rT2);

    EXPECT_EQ(one_status, TestObject::Status::Destroyed);
}

TEST(SPTR, CastIntrusive)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    skr::SPtr<TestSon> pC(SkrNew<TestSon>(status));
    skr::SPtr<TestObject> pP(pC);
    skr::SPtr<TestSon> pCC(skr::static_pointer_cast<TestSon>(pP));
    EXPECT_EQ(pCC->use_count(), 3);

    skr::SPtr<TestObject> pCC2 = (skr::SPtr<TestSon>)pCC; 
    EXPECT_EQ(pCC->use_count(), 4);
    
    skr::SPtr<TestObject> pCC3 = std::move(pCC); 
    EXPECT_EQ(pCC3->use_count(), 4);
}

TEST(SPTR, CastIntrusive2)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    skr::SPtr<TestSon> pC(SkrNew<TestSon>(status));
    skr::SPtr<TestObject> pP(pC);
    skr::SPtr<TestSon> pCC(skr::static_pointer_cast<TestSon>(pP));
    EXPECT_EQ(pCC->use_count(), 3);

    skr::SPtr<TestObject> pCC2 = (skr::SPtr<TestSon>)pCC; 
    EXPECT_EQ(pCC->use_count(), 4);
    
    skr::SPtr<TestObject> pCC3 = std::move(pCC); 
    EXPECT_EQ(pCC3->use_count(), 4);
}

TEST(SPTR, WeakIntrusive)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    skr::SPtr<TestSon> pC(SkrNew<TestSon>(status));
    skr::SWeakPtr<TestSon> wpC(pC);
    skr::SWeakPtr<TestSon> wpC2 = {};
    EXPECT_EQ(wpC2, nullptr);
    wpC2 = pC;

    EXPECT_EQ(wpC->use_count(), 1);
    EXPECT_EQ(wpC2->use_count(), 1);

    auto pC2 = wpC2.lock();
    EXPECT_EQ(pC.get(), pC2.get());
    EXPECT_EQ(pC, pC2);
    EXPECT_EQ(pC2->use_count(), 2);
}

TEST(SPTR, WeakIntrusive2)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    skr::SPtr<TestSon> pC(SkrNew<TestSon>(status));
    skr::SWeakPtr<TestSon> wpC(pC);
    skr::SWeakPtr<TestSon> wpC2 = {};
    EXPECT_EQ(wpC2, nullptr);
    wpC2 = pC;

    EXPECT_EQ(wpC->use_count(), 1);
    EXPECT_EQ(wpC2->use_count(), 1);

    auto pC2 = wpC2.lock();
    EXPECT_EQ(pC.get(), pC2.get());
    EXPECT_EQ(pC, pC2);
    EXPECT_EQ(pC2->use_count(), 2);
}
