#include "test.hpp"

struct SPTRIntrusiveTests
{

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

TEST_CASE_METHOD(SPTRIntrusiveTests, "CopyIntrusive")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "CopyIntrusive2")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "SwapIntrusive")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "SwapIntrusive2")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "MoveIntrusive")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "MoveIntrusive2")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "CastIntrusive")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "CastIntrusive2")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "VoidPtrCastIntrusive")
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

TEST_CASE_METHOD(SPTRIntrusiveTests, "BoxedValue")
{
    skr::SBoxedPtr<uint32_t> upValue;
    {
        auto object = skr::SBoxedPtr<uint32_t>(SkrNew<skr::SBoxed<uint32_t>>((uint32_t)1u));
        auto value = object.get()->get();
        EXPECT_EQ(object->get_type(), skr::type::type_id<uint32_t>::get());
        EXPECT_EQ(*value, 1);
        *value = 2;
        EXPECT_EQ(object->value, 2);
        upValue.reset(skr::box(value));
    }
    EXPECT_EQ(upValue->value, 2);
}