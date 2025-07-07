# SakuraEngine 任务系统

## 概述

SakuraEngine 的任务系统基于 Fiber（纤程）提供高性能的并行计算框架。系统默认使用 Google 的 marl 库作为后端，采用工作窃取调度策略，支持大规模并发任务执行。Fiber 比线程更轻量，允许在用户态进行上下文切换，特别适合游戏引擎中的细粒度并行任务。

## 核心概念

### Fiber 任务模型

Fiber 是比线程更轻量的执行单元：

- **用户态调度** - 避免系统调用开销
- **栈空间小** - 每个 Fiber 仅需几 KB 栈空间
- **快速切换** - 上下文切换成本极低
- **大规模并发** - 可创建数千个 Fiber

### 系统架构

```
┌─────────────────────────────────────────────────┐
│                Task System                       │
│  ┌──────────────────────────────────────────┐  │
│  │            Scheduler (marl)               │  │
│  │  ┌────────┐  ┌────────┐  ┌────────┐     │  │
│  │  │Worker 0│  │Worker 1│  │Worker N│     │  │
│  │  └────────┘  └────────┘  └────────┘     │  │
│  │       ↑           ↑           ↑          │  │
│  │       └───────────┴───────────┘          │  │
│  │           Work Stealing Queue             │  │
│  └──────────────────────────────────────────┘  │
│                                                 │
│  ┌──────────────────────────────────────────┐  │
│  │         Synchronization Primitives        │  │
│  │    counter_t    event_t    weak refs     │  │
│  └──────────────────────────────────────────┘  │
└─────────────────────────────────────────────────┘
```

## 基本使用

### 初始化任务系统

```cpp
#include <SkrTask/fib_task.hpp>

// 创建调度器
skr::task::scheduler_t scheduler;

// 绑定到当前线程（必须）
scheduler.bind();

// 设置工作线程数（默认为 CPU 核心数）
scheduler.set_worker_thread_count(std::thread::hardware_concurrency());

// 使用完毕后解绑
scheduler.unbind();
```

### 创建和执行任务

```cpp
// 简单任务调度
skr::task::schedule([](){ 
    printf("Task running on fiber!\n");
    
    // 执行计算密集型工作
    compute_something();
});

// 带名称的任务（用于调试）
skr::task::schedule([](){ 
    heavy_computation();
}, nullptr, "HeavyComputation");

// 绑定到事件的任务
auto event = skr::task::event_t::create();
skr::task::schedule([](){ 
    prepare_data();
}, event.get());

// 等待事件完成
event->wait();
```

### 同步原语

#### 计数器 (Counter)

```cpp
// 创建计数器
auto counter = skr::task::counter_t::create(0);

// 提交多个任务
const int task_count = 10;
for (int i = 0; i < task_count; ++i) {
    counter->add(1);  // 增加计数
    
    skr::task::schedule([counter, i]() {
        // 处理任务
        process_item(i);
        
        // 完成后减少计数
        counter->decrement();
    });
}

// 等待所有任务完成
counter->wait();

// 反向计数模式（用于生产者-消费者）
auto inverse_counter = skr::task::counter_t::create(0, true);
inverse_counter->add(5);  // 设置期望完成数
// ... 其他线程会增加计数直到达到 5
inverse_counter->wait();
```

#### 事件 (Event)

```cpp
// 创建手动重置事件
auto event = skr::task::event_t::create();

// 任务 1：等待事件
skr::task::schedule([event]() {
    printf("Waiting for signal...\n");
    event->wait();
    printf("Signal received!\n");
});

// 任务 2：触发事件
skr::task::schedule([event]() {
    // 准备数据
    prepare_data();
    
    // 触发事件
    event->signal();
});

// 清除事件状态
event->clear();
```

#### 弱引用

```cpp
// 弱引用避免循环依赖
auto strong_counter = skr::task::counter_t::create(0);
skr::task::weak_counter_t weak_counter = strong_counter;

// 在任务中安全使用
skr::task::schedule([weak_counter]() {
    // 尝试获取强引用
    if (auto counter = weak_counter.lock()) {
        counter->decrement();
    }
    // 如果对象已销毁，lock() 返回空
});
```

## 并行编程模式

### parallel_for - 数据并行

```cpp
#include <SkrTask/parallel_for.hpp>

// 并行处理数组
std::vector<float> data(10000);

skr::task::parallel_for(
    data.begin(), 
    data.end(),
    256,  // 批大小
    [](auto begin, auto end) {
        // 处理 [begin, end) 范围的数据
        for (auto it = begin; it != end; ++it) {
            *it = std::sqrt(*it);
        }
    },
    64    // 内联阈值：小于此大小直接执行
);

// 使用索引版本
skr::task::parallel_for(
    0, 
    data.size(),
    256,  // 批大小
    [&data](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i) {
            data[i] = process_element(data[i]);
        }
    }
);

// 自定义调度器和计数器
auto counter = skr::task::counter_t::create(0);
skr::task::parallel_for(
    items.begin(), 
    items.end(),
    100,
    process_batch,
    50,
    counter.get()  // 使用指定的计数器
);
counter->wait();  // 等待完成
```

### concurrent - 固定线程并发

```cpp
// 使用固定数量的任务处理数据
// 适合减少任务切换开销的场景
skr::task::concurrent(
    large_dataset.begin(),
    large_dataset.end(),
    1,    // 每个任务最少处理的元素数
    [](auto begin, auto end) {
        // 这个 lambda 只会在固定数量的任务中执行
        // 任务数 = min(CPU核心数, 数据量)
        for (auto it = begin; it != end; ++it) {
            expensive_computation(*it);
        }
    },
    1     // 内联阈值
);

// 异步版本 - 不阻塞当前线程
auto counter = skr::task::async_concurrent(
    items.begin(),
    items.end(),
    1,
    [](auto begin, auto end) {
        process_items(begin, end);
    }
);

// 继续其他工作
do_other_work();

// 稍后等待完成
counter->wait();
```

## 高级特性

### Fiber 管理

```cpp
// 获取当前 Fiber
void* current = skr::task::current_fiber();

// 等待条件满足
skr::task::wait(
    true,  // pin to thread
    [&]() { return data_ready; }
);

// 主动让出执行
skr::task::schedule([](){});  // 调度空任务相当于 yield
```

### 任务依赖管理

```cpp
// 使用计数器构建依赖关系
class TaskGraph {
    struct Node {
        skr::task::counter_t counter;
        std::function<void()> task;
        std::vector<Node*> dependents;
    };
    
public:
    void execute() {
        // 执行没有依赖的任务
        for (auto& node : roots) {
            execute_node(node);
        }
        
        // 等待所有任务完成
        for (auto& node : all_nodes) {
            node->counter->wait();
        }
    }
    
private:
    void execute_node(Node* node) {
        skr::task::schedule([node]() {
            // 执行任务
            node->task();
            
            // 触发依赖任务
            for (auto* dependent : node->dependents) {
                dependent->counter->decrement();
            }
        });
    }
};
```

### 与 ECS 系统集成

```cpp
// ECS 系统中的并行查询执行
void parallel_ecs_update(sugoi_storage_t* storage) {
    // 记录当前主线程 Fiber
    storage->main_thread_fiber = skr::task::current_fiber();
    
    // 创建同步计数器
    auto counter = skr::task::counter_t::create(0);
    
    // 并行处理 chunks
    sugoiJ_schedule_ecs(
        query,
        256,  // 批大小
        [](void* u, sugoi_query_t* query, 
           sugoi_chunk_view_t* view, 
           uint32_t start, uint32_t end) {
            
            // 在 Fiber 中处理数据
            auto positions = view->get_component_array<Position>();
            auto velocities = view->get_component_array<Velocity>();
            
            for (uint32_t i = start; i < end; ++i) {
                positions[i] += velocities[i] * delta_time;
            }
        },
        nullptr,           // userdata
        nullptr,           // init
        nullptr,           // teardown
        nullptr,           // resources
        counter.get()      // 同步计数器
    );
    
    // 等待查询完成
    counter->wait();
}
```

## 性能优化

### 批大小调优

```cpp
// 根据工作负载选择合适的批大小
template<typename Iter, typename Func>
void optimized_parallel_for(Iter begin, Iter end, Func f) {
    const size_t total = std::distance(begin, end);
    const size_t thread_count = skr::task::scheduler_t::thread_count();
    
    // 计算最优批大小
    size_t batch_size;
    if (is_memory_bound<Func>()) {
        // 内存密集型：较大批次减少调度开销
        batch_size = total / (thread_count * 2);
    } else {
        // 计算密集型：较小批次提高负载均衡
        batch_size = total / (thread_count * 8);
    }
    
    // 限制批大小范围
    batch_size = std::max(batch_size, size_t(16));
    batch_size = std::min(batch_size, size_t(1024));
    
    skr::task::parallel_for(begin, end, batch_size, f);
}
```

### 减少同步开销

```cpp
// 使用线程本地存储减少同步
class ParallelAccumulator {
    struct ThreadLocalData {
        alignas(64) double sum = 0;  // 缓存行对齐
        char padding[64 - sizeof(double)];
    };
    
    std::vector<ThreadLocalData> thread_data;
    
public:
    ParallelAccumulator() 
        : thread_data(skr::task::scheduler_t::thread_count()) {}
    
    void accumulate(const std::vector<double>& data) {
        skr::task::parallel_for(
            0, data.size(), 256,
            [&](size_t begin, size_t end) {
                // 获取线程本地存储
                auto tid = skr::task::worker_thread_index();
                auto& local = thread_data[tid];
                
                // 无锁累加
                for (size_t i = begin; i < end; ++i) {
                    local.sum += data[i];
                }
            }
        );
    }
    
    double get_sum() {
        double total = 0;
        for (auto& td : thread_data) {
            total += td.sum;
        }
        return total;
    }
};
```

### 任务池化

```cpp
// 复用任务对象减少分配
class TaskPool {
    skr::task::counter_t active_tasks;
    std::vector<std::function<void()>> task_cache;
    std::mutex cache_mutex;
    
public:
    template<typename F>
    void execute(F&& task) {
        std::function<void()> cached_task;
        
        // 尝试从缓存获取
        {
            std::lock_guard<std::mutex> lock(cache_mutex);
            if (!task_cache.empty()) {
                cached_task = std::move(task_cache.back());
                task_cache.pop_back();
            }
        }
        
        // 复用或创建新任务
        if (cached_task) {
            cached_task = std::forward<F>(task);
        } else {
            cached_task = std::function<void()>(std::forward<F>(task));
        }
        
        active_tasks->add(1);
        
        skr::task::schedule([this, task = std::move(cached_task)]() mutable {
            task();
            
            // 清理并回收
            task = nullptr;
            {
                std::lock_guard<std::mutex> lock(cache_mutex);
                if (task_cache.size() < 100) {
                    task_cache.push_back(std::move(task));
                }
            }
            
            active_tasks->decrement();
        });
    }
    
    void wait_all() {
        active_tasks->wait();
    }
};
```

## 最佳实践

### DO - 推荐做法

1. **选择合适的并行模式**
   ```cpp
   // 数据并行：使用 parallel_for
   skr::task::parallel_for(data.begin(), data.end(), 256, process);
   
   // 减少开销：使用 concurrent
   skr::task::concurrent(items.begin(), items.end(), 1, compute);
   
   // 异步执行：使用 async_concurrent
   auto counter = skr::task::async_concurrent(...);
   ```

2. **合理设置批大小**
   ```cpp
   // 考虑缓存局部性和负载均衡
   const size_t cache_line_elements = 64 / sizeof(Element);
   const size_t batch_size = cache_line_elements * 16;  // 16 个缓存行
   ```

3. **使用弱引用避免循环依赖**
   ```cpp
   // 在可能形成循环的地方使用弱引用
   skr::task::weak_counter_t weak = strong_counter;
   ```

### DON'T - 避免做法

1. **避免过小的任务粒度**
   ```cpp
   // 不好：任务太小，调度开销大于执行时间
   for (int i = 0; i < 1000000; ++i) {
       skr::task::schedule([i]() { array[i]++; });
   }
   
   // 好：批量处理
   skr::task::parallel_for(0, 1000000, 1000, [&](size_t b, size_t e) {
       for (size_t i = b; i < e; ++i) array[i]++;
   });
   ```

2. **避免在任务中阻塞**
   ```cpp
   // 不好：阻塞工作线程
   skr::task::schedule([]() {
       std::this_thread::sleep_for(1s);  // 阻塞整个工作线程！
   });
   
   // 好：使用事件等待
   event->wait();  // Fiber 会让出执行
   ```

3. **不要忘记 bind/unbind**
   ```cpp
   // 必须在使用调度器的线程上调用
   scheduler.bind();
   // ... 使用调度器
   scheduler.unbind();  // 清理线程本地存储
   ```

## 调试技巧

### 任务命名和追踪

```cpp
// 为任务添加有意义的名称
skr::task::schedule(
    []() { update_physics(); }, 
    nullptr, 
    "PhysicsUpdate"
);

// 使用性能分析标记
skr::task::schedule([]() {
    SKR_PROFILE_SCOPE("AI::UpdateBehavior");
    update_ai_behavior();
});
```

### 死锁检测

```cpp
// 使用超时检测潜在死锁
bool wait_with_timeout(skr::task::counter_t counter, double seconds) {
    auto start = std::chrono::steady_clock::now();
    
    while (counter->value() > 0) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::duration<double>(seconds)) {
            SKR_LOG_ERROR(u8"Possible deadlock detected!");
            return false;
        }
        
        // 短暂让出执行
        skr::task::schedule([](){});
    }
    
    return true;
}
```

### 统计信息

```cpp
// 监控任务系统状态
struct TaskStats {
    std::atomic<size_t> tasks_created{0};
    std::atomic<size_t> tasks_completed{0};
    std::atomic<size_t> total_wait_time{0};
    
    void print_report() {
        SKR_LOG_INFO(u8"=== Task System Stats ===");
        SKR_LOG_INFO(u8"Tasks created: %zu", tasks_created.load());
        SKR_LOG_INFO(u8"Tasks completed: %zu", tasks_completed.load());
        SKR_LOG_INFO(u8"Avg wait time: %.2f ms", 
            total_wait_time.load() / (double)tasks_completed.load());
    }
};
```

## 总结

SakuraEngine 的 Fiber 任务系统提供了轻量级、高性能的并行计算框架。通过基于 marl 的工作窃取调度器和丰富的同步原语，系统能够高效处理游戏引擎中的各种并行任务。合理使用 parallel_for、concurrent 等并行模式，配合适当的批大小和同步策略，可以充分发挥多核 CPU 的计算能力。