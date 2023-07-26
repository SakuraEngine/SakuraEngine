// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "marl_test.h"

#include "marl/containers.h"
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/waitgroup.h"

#include <atomic>

TEST_CASE_METHOD(WithoutBoundScheduler, "Destruct") {
  SUBCASE("SchedulerConstructAndDestruct") {
    auto scheduler = std::unique_ptr<marl::Scheduler>(
        new marl::Scheduler(marl::Scheduler::Config()));
  }

  SUBCASE("SchedulerBindGetUnbind") {
    auto scheduler = std::unique_ptr<marl::Scheduler>(
        new marl::Scheduler(marl::Scheduler::Config()));
    scheduler->bind();
    auto got = marl::Scheduler::get();
    ASSERT_EQ(scheduler.get(), got);
    scheduler->unbind();
    got = marl::Scheduler::get();
    ASSERT_EQ(got, nullptr);
  }

  SUBCASE("CheckConfig") {
    marl::Scheduler::Config cfg;
    cfg.setAllocator(allocator).setWorkerThreadCount(10);

    auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));

    auto gotCfg = scheduler->config();
    ASSERT_EQ(gotCfg.allocator, allocator);
    ASSERT_EQ(gotCfg.workerThread.count, 10);
  }

  SUBCASE("TasksOnlyScheduledOnWorkerThreads") {
    marl::Scheduler::Config cfg;
    cfg.setWorkerThreadCount(8);

    auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));
    scheduler->bind();
    defer(scheduler->unbind());

    std::mutex mutex;
    marl::containers::unordered_set<std::thread::id> threads(allocator);
    marl::WaitGroup wg;
    for (int i = 0; i < 10000; i++) {
      wg.add(1);
      marl::schedule([&mutex, &threads, wg] {
        defer(wg.done());
        std::unique_lock<std::mutex> lock(mutex);
        threads.emplace(std::this_thread::get_id());
      });
    }
    wg.wait();

    ASSERT_LE(threads.size(), 8U);
    ASSERT_EQ(threads.count(std::this_thread::get_id()), 0U);
  }

  // Test that a marl::Scheduler *with dedicated worker threads* can be used
  // without first binding to the scheduling thread.
  SUBCASE("ScheduleMTWWithNoBind") {
    marl::Scheduler::Config cfg;
    cfg.setWorkerThreadCount(8);
    auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));

    marl::WaitGroup wg;
    for (int i = 0; i < 100; i++) {
      wg.add(1);

      marl::Event event;
      scheduler->enqueue(marl::Task([event, wg] {
        event.wait();  // Test that tasks can wait on other tasks.
        wg.done();
      }));

      scheduler->enqueue(marl::Task([event, &scheduler] {
        // Despite the main thread never binding the scheduler, the scheduler
        // should be automatically bound to worker threads.
        ASSERT_EQ(marl::Scheduler::get(), scheduler.get());

        event.signal();
      }));
    }

    // As the scheduler has not been bound to the main thread, the wait() call
    // here will block **without** fiber yielding.
    wg.wait();
  }

  // Test that a marl::Scheduler *without dedicated worker threads* cannot be used
  // without first binding to the scheduling thread.
  SUBCASE("ScheduleSTWWithNoBind") {
    marl::Scheduler::Config cfg;
    auto scheduler = std::unique_ptr<marl::Scheduler>(new marl::Scheduler(cfg));
  }
}