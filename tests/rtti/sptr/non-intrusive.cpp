#include "test.hpp"

struct TestStruct
{
    enum Status
    {
        Uninitialized,
        Created,
        Destroyed
    };

    TestStruct(Status& code)
        : code(code)
    {
        code = Status::Created;
    }
    virtual ~TestStruct()
    {
        code = Status::Destroyed;
    }
    Status& code;
};

struct TestDerived : public TestStruct
{
    TestDerived(TestStruct::Status& code)
        : TestStruct(code)
    {
        
    }
    virtual ~TestDerived() = default;
};

TEST(SPTR, CopyNonIntrusive)
{
    TestStruct::Status status;
    {
        skr::SPtr<TestStruct> mTestStruct(SkrNew<TestStruct>(status));
        EXPECT_EQ(status, TestStruct::Status::Created);
        EXPECT_EQ(mTestStruct.use_count(), 1);
        skr::SPtr<TestStruct> mCopy(mTestStruct);
        EXPECT_EQ(mTestStruct.use_count(), 2);
        auto mCopy1 = mTestStruct;
        EXPECT_EQ(mTestStruct.use_count(), 3);
    }
    EXPECT_EQ(status, TestStruct::Status::Destroyed);
}

TEST(SPTR, SwapNonIntrusive)
{
    auto one_status = TestStruct::Status::Uninitialized;
    auto another_status = TestStruct::Status::Uninitialized;
    auto mOneStruct = skr::SPtr<TestStruct>::Create(one_status);
    {
        auto mAnotherStruct = skr::SPtr<TestStruct>::Create(another_status);
        mOneStruct = mAnotherStruct;     
        EXPECT_EQ(one_status, TestStruct::Status::Destroyed);
    }
    EXPECT_EQ(another_status, TestStruct::Status::Created);
}

TEST(SPTR, MoveNonIntrusive)
{
    auto one_status = TestStruct::Status::Uninitialized;
    auto another_status = TestStruct::Status::Uninitialized;
    skr::SPtr<TestStruct> rT1(SkrNew<TestStruct>(one_status));
    skr::SPtr<TestDerived> rT2(SkrNew<TestDerived>(another_status));  // default ctor uses 0

    EXPECT_EQ(rT1.use_count(), 1);
    EXPECT_EQ(rT2.use_count(), 1);

    rT1 = std::move(rT2);

    EXPECT_EQ(one_status, TestStruct::Status::Destroyed);
}

TEST(SPTR, CastNonIntrusive)
{
    TestStruct::Status status;
    skr::SPtr<TestDerived> pC(SkrNew<TestDerived>(status));
    skr::SPtr<TestStruct> pP(pC);
    skr::SPtr<TestDerived> pCC(skr::static_pointer_cast<TestDerived>(pP));
    EXPECT_EQ(pCC.use_count(), 3);

    skr::SPtr<TestStruct> pCC2 = (skr::SPtr<TestDerived>)pCC; 
    EXPECT_EQ(pCC.use_count(), 4);
    
    skr::SPtr<TestStruct> pCC3 = std::move(pCC); 
    EXPECT_EQ(pCC3.use_count(), 4);
}

TEST(SPTR, WeakNonIntrusive)
{
    TestStruct::Status status;
    skr::SWeakPtr<TestStruct> mWeak = {};
    {
        skr::SPtr<TestStruct> mTestStruct(SkrNew<TestStruct>(status));
        EXPECT_EQ(status, TestStruct::Status::Created);
        skr::SWeakPtr<TestStruct> mWeak2(mTestStruct);
        // EXPECT_EQ(mWeak2.lock(), mTestStruct.get());
    }
    EXPECT_EQ(status, TestStruct::Status::Destroyed);
}