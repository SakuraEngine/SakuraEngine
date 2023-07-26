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
#pragma once
#include <iostream>
#include "marl/scheduler.h"
#include "SkrTestFramework/framework.hpp"

// SchedulerParams holds Scheduler construction parameters for testing.
struct SchedulerParams {
  int numWorkerThreads = 0;

  friend std::ostream& operator<<(std::ostream& os,
                                  const SchedulerParams& params) {
    return os << "SchedulerParams{"
              << "numWorkerThreads: " << params.numWorkerThreads << "}";
  }
};

// WithoutBoundScheduler is a test fixture that does not bind a scheduler.
class WithoutBoundScheduler {
 public:
  WithoutBoundScheduler() {
    allocator = new marl::TrackedAllocator(marl::Allocator::Default);
  }

  ~WithoutBoundScheduler() {
    auto stats = allocator->stats();
    EXPECT_EQ(stats.numAllocations(), 0U);
    EXPECT_EQ(stats.bytesAllocated(), 0U);
    delete allocator;
  }

  marl::TrackedAllocator* allocator = nullptr;
};

struct WithBoundSchedulerBase
{
  void TestCondVars();
  void TestCalls();
  void TestEvents();
  void TestUnboundedPool();
  void TestTicket();

  marl::TrackedAllocator* allocator = nullptr;
};

// WithBoundScheduler is a parameterized test fixture that performs tests with
// a bound scheduler using a number of different configurations.
template<uint32_t N_THREADS>
class WithBoundScheduler : public WithBoundSchedulerBase
{
 public:
  WithBoundScheduler() {
    allocator = new marl::TrackedAllocator(marl::Allocator::Default);
    auto params = SchedulerParams{ N_THREADS };

    marl::Scheduler::Config cfg;
    cfg.setAllocator(allocator);
    cfg.setWorkerThreadCount(params.numWorkerThreads);
    cfg.setFiberStackSize(0x10000);

    auto scheduler = new marl::Scheduler(cfg);
    scheduler->bind();
  }

  ~WithBoundScheduler() {
    auto scheduler = marl::Scheduler::get();
    scheduler->unbind();
    delete scheduler;

    auto stats = allocator->stats();
    EXPECT_EQ(stats.numAllocations(), 0U);
    EXPECT_EQ(stats.bytesAllocated(), 0U);
    delete allocator;
  }
};
