#include "gtest/gtest.h"
#include "utils/make_zeroed.hpp"
#include "job/thread_job.hpp"

TEST(Job, JobQueue)
{
    auto jqDesc = make_zeroed<skr::JobQueueDesc>();
    jqDesc.thread_count = 2;
    jqDesc.priority = SKR_THREAD_NORMAL;
    auto jq = skr::JobQueue(&jqDesc);
    struct JI : public skr::JobItem
    {
        JI() : JobItem(u8"TestJob")
        {
            std::cout << "Ctor Test Job!" << std::endl;
        }
        skr::JobResult run() SKR_NOEXCEPT override
        {
            std::cout << "Hello Test Job!" << std::endl;
            return skr::JOB_RESULT_OK;
        }
        void finish(skr::JobResult result) SKR_NOEXCEPT override
        {
            std::cout << "Finish with result: " << result << std::endl;
        }
    } ji;
    jq.enqueue(&ji);
    jq.check();
    jq.wait_empty();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}