#include "test.hpp"

class SPTRIntrusive : public SPTRBase
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

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

TEST(SPTRIntrusive, CopyIntrusive)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    {
        auto mTestObject = skr::SObjectPtr<TestObject>::Create(status);
        EXPECT_EQ(mTestObject->rc, 1);
        skr::SObjectPtr<TestObject> mCopy(mTestObject);
        EXPECT_EQ(mTestObject->rc, 2);
        auto mCopy1 = mTestObject;
        EXPECT_EQ(mTestObject->rc, 3);
    }
    EXPECT_EQ(status, TestObject::Status::Destroyed);
}

TEST(SPTRIntrusive, CopyIntrusive2)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    {
        auto mTestObject = skr::SObjectPtr<TestObject>::Create(status);
        EXPECT_EQ(mTestObject->rc, 1);
        skr::SObjectPtr<TestObject> mCopy(mTestObject);
        EXPECT_EQ(mTestObject->rc, 2);
        auto mCopy1 = mTestObject;
        EXPECT_EQ(mTestObject->rc, 3);
    }
    EXPECT_EQ(status, TestObject::Status::Destroyed);
}

TEST(SPTRIntrusive, SwapIntrusive)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    auto mOneObject = skr::SObjectPtr<TestObject>::Create(one_status);
    {
        auto mAnotherObject = skr::SObjectPtr<TestObject>::Create(another_status);
        mOneObject = mAnotherObject;     
        EXPECT_EQ(one_status, TestObject::Status::Destroyed);
    }
    EXPECT_EQ(another_status, TestObject::Status::Hosted);
}

TEST(SPTRIntrusive, SwapIntrusive2)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    auto mOneObject = skr::SObjectPtr<TestObject>::Create(one_status);
    {
        auto mAnotherObject = skr::SObjectPtr<TestObject>::Create(another_status);
        mOneObject = mAnotherObject;     
        EXPECT_EQ(one_status, TestObject::Status::Destroyed);
    }
    EXPECT_EQ(another_status, TestObject::Status::Hosted);
}

TEST(SPTRIntrusive, MoveIntrusive)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    skr::SObjectPtr<TestObject> rT1(SkrNew<TestObject>(one_status));
    skr::SObjectPtr<TestSon> rT2(SkrNew<TestSon>(another_status)); // default ctor uses 0

    EXPECT_EQ(one_status, TestObject::Status::Hosted);
    EXPECT_EQ(another_status, TestObject::Status::Hosted);

    rT1 = std::move(rT2);

    EXPECT_EQ(one_status, TestObject::Status::Destroyed);
}

TEST(SPTRIntrusive, MoveIntrusive2)
{
    auto one_status = TestObject::Status::Uninitialized;
    auto another_status = TestObject::Status::Uninitialized;
    skr::SObjectPtr<TestObject> rT1(SkrNew<TestObject>(one_status));
    skr::SObjectPtr<TestSon> rT2(SkrNew<TestSon>(another_status)); // default ctor uses 0

    EXPECT_EQ(one_status, TestObject::Status::Hosted);
    EXPECT_EQ(another_status, TestObject::Status::Hosted);

    rT1 = std::move(rT2);

    EXPECT_EQ(one_status, TestObject::Status::Destroyed);
}

TEST(SPTRIntrusive, CastIntrusive)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    skr::SObjectPtr<TestSon> pC(SkrNew<TestSon>(status));
    skr::SObjectPtr<TestObject> pP(pC);
    skr::SObjectPtr<TestSon> pCC(skr::static_pointer_cast<TestSon>(pP));
    EXPECT_EQ(pCC->use_count(), 3);

    skr::SObjectPtr<TestObject> pCC2 = (skr::SObjectPtr<TestSon>)pCC; 
    EXPECT_EQ(pCC->use_count(), 4);
    
    skr::SObjectPtr<TestObject> pCC3 = std::move(pCC); 
    EXPECT_EQ(pCC3->use_count(), 4);
}

TEST(SPTRIntrusive, CastIntrusive2)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    skr::SObjectPtr<TestSon> pC(SkrNew<TestSon>(status));
    skr::SObjectPtr<TestObject> pP(pC);
    skr::SObjectPtr<TestSon> pCC(skr::static_pointer_cast<TestSon>(pP));
    EXPECT_EQ(pCC->use_count(), 3);

    skr::SObjectPtr<TestObject> pCC2 = (skr::SObjectPtr<TestSon>)pCC; 
    EXPECT_EQ(pCC->use_count(), 4);
    
    skr::SObjectPtr<TestObject> pCC3 = std::move(pCC); 
    EXPECT_EQ(pCC3->use_count(), 4);
}

TEST(SPTRIntrusive, VoidPtrCastIntrusive)
{
    TestObject::Status status = TestObject::Status::Uninitialized;
    {
        skr::SObjectPtr<void> pVC;
        {
            skr::SObjectPtr<TestSon> pC(SkrNew<TestSon>(status));
            pVC = pC;
            EXPECT_EQ(pC->use_count(), 2);
        }
        EXPECT_EQ(status, TestObject::Status::Hosted);
    }
    EXPECT_EQ(status, TestObject::Status::Destroyed);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}