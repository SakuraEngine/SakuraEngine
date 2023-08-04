// Copyright 2019 The Marl Authors
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

#include "marl/dag.h"
#include "marl/defer.h"
#include "marl/blockingcall.h"
#include "marl/parallelize.h"
#include "marl/waitgroup.h"

#include <mutex>
#include <string>

namespace {

struct Data {
  std::mutex mutex;
  std::vector<std::string> order;

  void push(std::string&& s) {
    std::unique_lock<std::mutex> lock(mutex);
    order.emplace_back(std::move(s));
  }
};

template <typename T>
std::vector<T> slice(const std::vector<T>& in, size_t from, size_t to) {
  return {in.begin() + from, in.begin() + to};
}

}  // namespace

void WithBoundSchedulerBase::TestCalls()
{
  SUBCASE("BlockingCallVoidReturn") {
    auto mutex = std::make_shared<std::mutex>();
    mutex->lock();

    marl::WaitGroup wg(100);
    for (int i = 0; i < 100; i++) {
      marl::schedule([=] {
        defer(wg.done());
        marl::blocking_call([=] {
          mutex->lock();
          defer(mutex->unlock());
        });
      });
    }

    mutex->unlock();
    wg.wait();
  }

  SUBCASE("BlockingCallIntReturn") {
    auto mutex = std::make_shared<std::mutex>();
    mutex->lock();

    marl::WaitGroup wg(100);
    std::atomic<int> n = {0};
    for (int i = 0; i < 100; i++) {
      marl::schedule([=, &n] {
        defer(wg.done());
        n += marl::blocking_call([=] {
          mutex->lock();
          defer(mutex->unlock());
          return i;
        });
      });
    }

    mutex->unlock();
    wg.wait();

    ASSERT_EQ(n.load(), 4950);
  }

  SUBCASE("BlockingCallSchedulesTask") {
    marl::WaitGroup wg(1);
    marl::schedule([=] {
      marl::blocking_call([=] { marl::schedule([=] { wg.done(); }); });
    });
    wg.wait();
  }

  SUBCASE("Parallelize") {
    bool a = false;
    bool b = false;
    bool c = false;
    marl::parallelize([&] { a = true; }, [&] { b = true; }, [&] { c = true; });
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    ASSERT_TRUE(c);
  }

  //  [A] --> [B] --> [C]                                                        |
  SUBCASE("DAGChainNoArg") {
    marl::DAG<>::Builder builder;

    Data data;
    builder.root()
        .then([&] { data.push("A"); })
        .then([&] { data.push("B"); })
        .then([&] { data.push("C"); });

    auto dag = builder.build();
    dag->run();

    EXPECT_EQ(data.order[0], "A");
    EXPECT_EQ(data.order[1], "B");
    EXPECT_EQ(data.order[2], "C");
  }

  //  [A] --> [B] --> [C]                                                        |
  SUBCASE("DAGChain") {
    marl::DAG<Data&>::Builder builder;

    builder.root()
        .then([](Data& data) { data.push("A"); })
        .then([](Data& data) { data.push("B"); })
        .then([](Data& data) { data.push("C"); });

    auto dag = builder.build();

    Data data;
    dag->run(data);

    EXPECT_EQ(data.order[0], "A");
    EXPECT_EQ(data.order[1], "B");
    EXPECT_EQ(data.order[2], "C");
  }

  //  [A] --> [B] --> [C]                                                        |
  SUBCASE("DAGRunRepeat") {
    marl::DAG<Data&>::Builder builder;

    builder.root()
        .then([](Data& data) { data.push("A"); })
        .then([](Data& data) { data.push("B"); })
        .then([](Data& data) { data.push("C"); });

    auto dag = builder.build();

    Data dataA, dataB;
    dag->run(dataA);
    dag->run(dataB);
    dag->run(dataA);

    EXPECT_EQ(dataA.order[0], "A");
    EXPECT_EQ(dataA.order[1], "B");
    EXPECT_EQ(dataA.order[2], "C");
    EXPECT_EQ(dataA.order[3], "A");
    EXPECT_EQ(dataA.order[4], "B");
    EXPECT_EQ(dataA.order[5], "C");

    EXPECT_EQ(dataB.order[0], "A");
    EXPECT_EQ(dataB.order[1], "B");
    EXPECT_EQ(dataB.order[2], "C");
  }

  //           /--> [A]                                                          |
  //  [root] --|--> [B]                                                          |
  //           \--> [C]                                                          |
  SUBCASE("DAGFanOutFromRoot") {
    marl::DAG<Data&>::Builder builder;

    auto root = builder.root();
    root.then([](Data& data) { data.push("A"); });
    root.then([](Data& data) { data.push("B"); });
    root.then([](Data& data) { data.push("C"); });

    auto dag = builder.build();

    Data data;
    dag->run(data);

    // TODO:
    // ASSERT_THAT(data.order, UnorderedElementsAre("A", "B", "C"));
  }

  //                /--> [A]                                                     |
  // [root] -->[N]--|--> [B]                                                     |
  //                \--> [C]                                                     |
  SUBCASE("DAGFanOutFromNonRoot") {
    marl::DAG<Data&>::Builder builder;

    auto root = builder.root();
    auto node = root.then([](Data& data) { data.push("N"); });
    node.then([](Data& data) { data.push("A"); });
    node.then([](Data& data) { data.push("B"); });
    node.then([](Data& data) { data.push("C"); });

    auto dag = builder.build();

    Data data;
    dag->run(data);

    ASSERT_EQ(data.order[0], "N");
    // TODO:
    // ASSERT_THAT(data.order, UnorderedElementsAre("N", "A", "B", "C"));
    // ASSERT_THAT(slice(data.order, 1, 4), UnorderedElementsAre("A", "B", "C"));
  }

  //          /--> [A0] --\        /--> [C0] --\        /--> [E0] --\            |
  // [root] --|--> [A1] --|-->[B]--|--> [C1] --|-->[D]--|--> [E1] --|-->[F]      |
  //                               \--> [C2] --/        |--> [E2] --|            |
  //                                                    \--> [E3] --/            |
  SUBCASE("DAGFanOutFanIn") {
    marl::DAG<Data&>::Builder builder;

    auto root = builder.root();
    auto a0 = root.then([](Data& data) { data.push("A0"); });
    auto a1 = root.then([](Data& data) { data.push("A1"); });

    auto b = builder.node([](Data& data) { data.push("B"); }, {a0, a1});

    auto c0 = b.then([](Data& data) { data.push("C0"); });
    auto c1 = b.then([](Data& data) { data.push("C1"); });
    auto c2 = b.then([](Data& data) { data.push("C2"); });

    auto d = builder.node([](Data& data) { data.push("D"); }, {c0, c1, c2});

    auto e0 = d.then([](Data& data) { data.push("E0"); });
    auto e1 = d.then([](Data& data) { data.push("E1"); });
    auto e2 = d.then([](Data& data) { data.push("E2"); });
    auto e3 = d.then([](Data& data) { data.push("E3"); });

    builder.node([](Data& data) { data.push("F"); }, {e0, e1, e2, e3});

    auto dag = builder.build();

    Data data;
    dag->run(data);

    // TODO:
    //ASSERT_THAT(data.order,
    //            UnorderedElementsAre("A0", "A1", "B", "C0", "C1", "C2", "D", "E0",
    //                                 "E1", "E2", "E3", "F"));
    //ASSERT_THAT(slice(data.order, 0, 2), UnorderedElementsAre("A0", "A1"));
    ASSERT_EQ(data.order[2], "B");
    //ASSERT_THAT(slice(data.order, 3, 6), UnorderedElementsAre("C0", "C1", "C2"));
    ASSERT_EQ(data.order[6], "D");
    //ASSERT_THAT(slice(data.order, 7, 11),
    //            UnorderedElementsAre("E0", "E1", "E2", "E3"));
    ASSERT_EQ(data.order[11], "F");
  }

  SUBCASE("WaitGroup_OneTask") {
    marl::WaitGroup wg(1);
    std::atomic<int> counter = {0};
    marl::schedule([&counter, wg] {
      counter++;
      wg.done();
    });
    wg.wait();
    ASSERT_EQ(counter.load(), 1);
  }

  SUBCASE("WaitGroup_10Tasks") {
    marl::WaitGroup wg(10);
    std::atomic<int> counter = {0};
    for (int i = 0; i < 10; i++) {
      marl::schedule([&counter, wg] {
        counter++;
        wg.done();
      });
    }
    wg.wait();
    ASSERT_EQ(counter.load(), 10);
  }

  SUBCASE("DestructWithPendingTasks") {
    std::atomic<int> counter = {0};
    for (int i = 0; i < 1000; i++) {
      marl::schedule([&] { counter++; });
    }

    auto scheduler = marl::Scheduler::get();
    scheduler->unbind();
    delete scheduler;

    // All scheduled tasks should be completed before the scheduler is destructed.
    ASSERT_EQ(counter.load(), 1000);

    // Rebind a new scheduler so WithBoundScheduler::TearDown() is happy.
    (new marl::Scheduler(marl::Scheduler::Config()))->bind();
  }

  SUBCASE("DestructWithPendingFibers") {
    std::atomic<int> counter = {0};

    marl::WaitGroup wg(1);
    for (int i = 0; i < 1000; i++) {
      marl::schedule([&] {
        wg.wait();
        counter++;
      });
    }

    // Schedule a task to unblock all the tasks scheduled above.
    // We assume that some of these tasks will not finish before the scheduler
    // destruction logic kicks in.
    marl::schedule([=] {
      wg.done();  // Ready, steady, go...
    });

    auto scheduler = marl::Scheduler::get();
    scheduler->unbind();
    delete scheduler;

    // All scheduled tasks should be completed before the scheduler is destructed.
    ASSERT_EQ(counter.load(), 1000);

    // Rebind a new scheduler so WithBoundScheduler::TearDown() is happy.
    (new marl::Scheduler(marl::Scheduler::Config()))->bind();
  }

  SUBCASE("ScheduleWithArgs") {
    std::string got;
    marl::WaitGroup wg(1);
    marl::schedule(
        [wg, &got](std::string s, int i, bool b) {
          got = "s: '" + s + "', i: " + std::to_string(i) +
                ", b: " + (b ? "true" : "false");
          wg.done();
        },
        "a string", 42, true);
    wg.wait();
    ASSERT_EQ(got, "s: 'a string', i: 42, b: true");
  }

  SUBCASE("FibersResumeOnSameThread") {
    marl::WaitGroup fence(1);
    marl::WaitGroup wg(1000);
    for (int i = 0; i < 1000; i++) {
      marl::schedule([=] {
        auto threadID = std::this_thread::get_id();
        fence.wait();
        ASSERT_EQ(threadID, std::this_thread::get_id());
        wg.done();
      });
    }
    // just to try and get some tasks to yield.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    fence.done();
    wg.wait();
  }

  SUBCASE("FibersResumeOnSameStdThread") {
    auto scheduler = marl::Scheduler::get();

    // on 32-bit OSs, excessive numbers of threads can run out of address space.
    constexpr auto num_threads = sizeof(void*) > 4 ? 1000 : 100;

    marl::WaitGroup fence(1);
    marl::WaitGroup wg(num_threads);

    marl::containers::vector<std::thread, 32> threads;
    for (int i = 0; i < num_threads; i++) {
      threads.emplace_back(std::thread([=] {
        scheduler->bind();
        defer(scheduler->unbind());

        auto threadID = std::this_thread::get_id();
        fence.wait();
        ASSERT_EQ(threadID, std::this_thread::get_id());
        wg.done();
      }));
    }
    // just to try and get some tasks to yield.
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    fence.done();
    wg.wait();

    for (auto& thread : threads) {
      thread.join();
    }
  }
}

TEST_CASE_METHOD(WithoutBoundScheduler, "WaitGroupDone") {
  marl::WaitGroup wg(2);  // Should not require a scheduler.
  wg.done();
  wg.done();
}

TEST_CASE_METHOD(WithBoundScheduler<0>, "TestCalls-0") { TestCalls(); };
TEST_CASE_METHOD(WithBoundScheduler<1>, "TestCalls-1") { TestCalls(); };
TEST_CASE_METHOD(WithBoundScheduler<2>, "TestCalls-2") { TestCalls(); };
TEST_CASE_METHOD(WithBoundScheduler<8>, "TestCalls-8") { TestCalls(); };
TEST_CASE_METHOD(WithBoundScheduler<32>, "TestCalls-32") { TestCalls(); };