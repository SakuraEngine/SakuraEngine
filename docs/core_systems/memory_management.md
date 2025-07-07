# SakuraEngine 内存管理系统

## 概述

SakuraEngine 的内存管理系统提供了高性能、类型安全的内存分配和管理功能。系统基于 Microsoft 的 mimalloc 构建，提供了统一的分配接口、智能指针、内存池以及完善的调试支持。

## 核心组件

### 系统架构

```
┌─────────────────────────────────────────────────────┐
│                  高层抽象                           │
│   SkrNew/Delete    RC/SP智能指针    容器分配器     │
├─────────────────────────────────────────────────────┤
│                  中间层                             │
│   sakura_malloc    内存池系统    对齐分配          │
├─────────────────────────────────────────────────────┤
│                  基础层                             │
│   mimalloc         OS分配器      Tracy追踪         │
└─────────────────────────────────────────────────────┘
```

### 内存分配接口

```cpp
// 基础内存分配
void* sakura_malloc(size_t size);
void* sakura_calloc(size_t count, size_t size);
void* sakura_realloc(void* ptr, size_t size);
void sakura_free(void* ptr);

// 对齐内存分配
void* sakura_malloc_aligned(size_t size, size_t alignment);
void sakura_free_aligned(void* ptr, size_t alignment);

// 命名内存分配（用于分类追踪）
void* sakura_malloc_named(size_t size, const char* name);
void* sakura_calloc_named(size_t count, size_t size, const char* name);

// C++ 对象分配
template<typename T, typename... Args>
T* SkrNew(Args&&... args);

template<typename T>
void SkrDelete(T* ptr);

// 数组分配
template<typename T>
T* SkrNewArray(size_t count);

template<typename T>
void SkrDeleteArray(T* ptr);
```

## 智能指针系统

### RC (Reference Counted) 系统

RC 系统提供侵入式引用计数智能指针：

```cpp
// 定义支持 RC 的类
class MyResource {
    SKR_RC_IMPL(MyResource)  // 添加引用计数支持
public:
    void do_something();
};

// 使用 RC 智能指针
void example_rc() {
    // 创建共享所有权对象
    RC<MyResource> resource = RC<MyResource>::New();
    
    // 自动引用计数管理
    RC<MyResource> copy = resource;  // 引用计数 +1
    
    // 独占所有权
    RCUnique<MyResource> unique = RCUnique<MyResource>::New();
    
    // 转移到共享所有权
    RC<MyResource> shared = std::move(unique);
    
    // 弱引用（不增加引用计数）
    RCWeak<MyResource> weak = resource;
    
    // 从弱引用恢复强引用
    if (auto locked = weak.lock()) {
        locked->do_something();
    }
}

// 自定义删除行为
class CustomResource {
    SKR_RC_IMPL(CustomResource)
    
    void on_free() override {
        // 自定义清理逻辑
        cleanup_resources();
    }
};
```

### SP (Shared Pointer) 系统

SP 系统提供非侵入式共享指针：

```cpp
// 任何类型都可以使用 SP
struct Data {
    int value;
    float score;
};

void example_sp() {
    // 创建共享指针
    SP<Data> data = skr::make_shared<Data>();
    data->value = 42;
    
    // 复制共享所有权
    SP<Data> copy = data;
    
    // 独占指针
    UPtr<Data> unique = skr::make_unique<Data>();
    
    // 弱引用
    SPWeak<Data> weak = data;
    
    // 自定义删除器
    SP<FILE> file(fopen("test.txt", "r"), [](FILE* f) {
        if (f) fclose(f);
    });
}

// 从原始指针创建（需要小心使用）
Data* raw = new Data();
SP<Data> managed(raw);  // 接管所有权
```

### 智能指针选择指南

| 特性 | RC | SP |
|------|----|----|
| 侵入性 | 需要继承/宏 | 无需修改类 |
| 内存开销 | 对象内嵌计数器 | 外部控制块 |
| 性能 | 略优 | 略差 |
| 灵活性 | 支持自定义删除 | 更灵活 |
| 使用场景 | 引擎核心对象 | 通用场景 |

## 内存池系统

### 创建和使用内存池

```cpp
// 创建内存池
SSysMemPoolDesc pool_desc = {
    .name = "RenderPool",
    .arena_size = 64 * 1024 * 1024,  // 64MB
    .enable_large_pages = false
};
SSysMemoryPool* pool = sysmem_pool_create(&pool_desc);

// 从池中分配内存
void* buffer = sysmem_pool_malloc(pool, 1024);

// 使用对齐分配
void* aligned = sysmem_pool_malloc_aligned(pool, 4096, 64);

// 清理内存池
sysmem_pool_reset(pool);  // 重置但保留内存
sysmem_pool_destroy(pool); // 完全销毁
```

### 线程本地内存池

```cpp
// 为每个线程创建独立的内存池
thread_local SSysMemoryPool* tls_pool = nullptr;

void init_thread_pool() {
    SSysMemPoolDesc desc = {
        .name = "ThreadPool",
        .arena_size = 16 * 1024 * 1024  // 16MB
    };
    tls_pool = sysmem_pool_create(&desc);
}

// 使用线程本地池
void* thread_alloc(size_t size) {
    return sysmem_pool_malloc(tls_pool, size);
}
```

## 容器分配器

### 标准分配器

```cpp
// 默认分配器
struct SkrAllocator {
    template <typename T>
    static T* alloc(size_t size) {
        return (T*)sakura_malloc_aligned(size, alignof(T));
    }
    
    template <typename T>
    static void free(T* p) {
        sakura_free(p);
    }
    
    template <typename T>
    static T* realloc(T* p, size_t size) {
        return (T*)sakura_realloc(p, size);
    }
    
    static constexpr bool support_realloc = true;
};

// 使用自定义分配器的容器
skr::vector<int, SkrAllocator> numbers;
skr::string<SkrAllocator> text;
```

### STL 适配器

```cpp
// STL 容器适配器
template <typename T>
using skr_stl_allocator = /* 实现细节 */;

// 使用 SakuraEngine 分配器的 STL 容器
std::vector<int, skr_stl_allocator<int>> stl_vec;
std::string<char, std::char_traits<char>, skr_stl_allocator<char>> stl_str;
```

## 内存调试和分析

### Tracy 集成

```cpp
// 启用内存追踪
#define SKR_MEMORY_TRACKING 1

// 所有分配都会被追踪
void* ptr = sakura_malloc(1024);  // 自动记录到 Tracy

// 命名分配便于分类
void* render_data = sakura_malloc_named(4096, "Render");
void* physics_data = sakura_malloc_named(2048, "Physics");

// 在 Tracy 中可以看到：
// - 分配大小和时间
// - 调用栈
// - 内存使用统计
```

### 内存泄漏检测

```cpp
// 使用 RAII 包装器自动检测泄漏
class MemoryLeakDetector {
    size_t initial_allocated;
    
public:
    MemoryLeakDetector() {
        initial_allocated = get_total_allocated();
    }
    
    ~MemoryLeakDetector() {
        size_t current = get_total_allocated();
        if (current > initial_allocated) {
            SKR_LOG_WARN(u8"Memory leak detected: %zu bytes", 
                        current - initial_allocated);
        }
    }
};

// 在作用域中检测
{
    MemoryLeakDetector detector;
    // 执行可能泄漏的代码
    run_tests();
}  // 自动检测泄漏
```

### 内存统计

```cpp
// 获取内存使用统计
struct MemoryStats {
    size_t total_allocated;
    size_t total_freed;
    size_t current_usage;
    size_t peak_usage;
    size_t allocation_count;
};

MemoryStats get_memory_stats() {
    return {
        .total_allocated = mi_stats_get_total_allocated(),
        .total_freed = mi_stats_get_total_freed(),
        .current_usage = mi_stats_get_current_allocated(),
        .peak_usage = mi_stats_get_peak_allocated(),
        .allocation_count = mi_stats_get_allocation_count()
    };
}

// 打印内存报告
void print_memory_report() {
    auto stats = get_memory_stats();
    SKR_LOG_INFO(u8"=== Memory Report ===");
    SKR_LOG_INFO(u8"Current: %.2f MB", stats.current_usage / 1048576.0);
    SKR_LOG_INFO(u8"Peak: %.2f MB", stats.peak_usage / 1048576.0);
    SKR_LOG_INFO(u8"Allocations: %zu", stats.allocation_count);
}
```

## 高级特性

### 自定义内存分配策略

```cpp
// 实现自定义分配器
class StackAllocator {
    uint8_t* buffer;
    size_t size;
    size_t offset;
    
public:
    StackAllocator(size_t size) 
        : buffer((uint8_t*)sakura_malloc(size))
        , size(size)
        , offset(0) {}
    
    void* allocate(size_t n, size_t alignment = 8) {
        offset = (offset + alignment - 1) & ~(alignment - 1);
        if (offset + n > size) return nullptr;
        
        void* ptr = buffer + offset;
        offset += n;
        return ptr;
    }
    
    void reset() { offset = 0; }
    
    ~StackAllocator() {
        sakura_free(buffer);
    }
};
```

### 内存映射文件

```cpp
// 大文件内存映射
class MemoryMappedFile {
    void* mapping;
    size_t size;
    
public:
    MemoryMappedFile(const char* path) {
        // 平台相关的内存映射实现
        #ifdef _WIN32
            // Windows CreateFileMapping
        #else
            // POSIX mmap
        #endif
    }
    
    void* data() { return mapping; }
    size_t size() { return size; }
};
```

### 对象池

```cpp
// 高性能对象池
template<typename T>
class ObjectPool {
    struct Block {
        alignas(T) char storage[sizeof(T)];
        Block* next;
    };
    
    Block* free_list;
    std::vector<std::unique_ptr<Block[]>> blocks;
    size_t block_size;
    
public:
    ObjectPool(size_t block_size = 1024) 
        : free_list(nullptr)
        , block_size(block_size) {
        grow();
    }
    
    template<typename... Args>
    T* acquire(Args&&... args) {
        if (!free_list) grow();
        
        Block* block = free_list;
        free_list = free_list->next;
        
        return new(block) T(std::forward<Args>(args)...);
    }
    
    void release(T* obj) {
        obj->~T();
        
        Block* block = reinterpret_cast<Block*>(obj);
        block->next = free_list;
        free_list = block;
    }
    
private:
    void grow() {
        auto new_block = std::make_unique<Block[]>(block_size);
        
        for (size_t i = 0; i < block_size - 1; ++i) {
            new_block[i].next = &new_block[i + 1];
        }
        new_block[block_size - 1].next = free_list;
        
        free_list = &new_block[0];
        blocks.push_back(std::move(new_block));
    }
};
```

## 最佳实践

### DO - 推荐做法

1. **使用智能指针管理生命周期**
   ```cpp
   // 好：自动管理
   RC<Resource> resource = RC<Resource>::New();
   
   // 避免：手动管理
   Resource* resource = new Resource();
   // ... 容易忘记 delete
   ```

2. **选择合适的分配策略**
   ```cpp
   // 短生命周期：栈分配器
   StackAllocator frame_alloc(1024 * 1024);
   
   // 频繁创建销毁：对象池
   ObjectPool<Particle> particle_pool;
   
   // 大块连续内存：内存池
   SSysMemoryPool* level_pool;
   ```

3. **使用命名分配追踪内存**
   ```cpp
   // 便于分析和调试
   void* ai_data = sakura_malloc_named(size, "AI");
   void* render_data = sakura_malloc_named(size, "Render");
   ```

### DON'T - 避免做法

1. **避免内存泄漏**
   ```cpp
   // 不好：异常时泄漏
   Resource* res = new Resource();
   risky_operation();  // 可能抛出异常
   delete res;
   
   // 好：RAII 保证释放
   auto res = skr::make_unique<Resource>();
   risky_operation();
   ```

2. **不要混用分配器**
   ```cpp
   // 错误：不同分配器
   void* ptr = malloc(100);
   sakura_free(ptr);  // 错误！
   
   // 正确：配对使用
   void* ptr = sakura_malloc(100);
   sakura_free(ptr);
   ```

## 性能优化

### 减少分配次数

```cpp
// 预分配容器容量
skr::vector<Entity> entities;
entities.reserve(1000);  // 避免多次重分配

// 复用对象
class ParticleSystem {
    ObjectPool<Particle> pool;
    
    void emit() {
        auto* p = pool.acquire();
        // 使用粒子
    }
    
    void recycle(Particle* p) {
        pool.release(p);
    }
};
```

### 内存对齐优化

```cpp
// 确保缓存行对齐
struct alignas(64) CacheLineData {
    std::atomic<int> counter;
    char padding[60];  // 填充到 64 字节
};

// SIMD 对齐
alignas(16) float vectors[4];
```

### 批量操作

```cpp
// 批量分配
struct BatchAllocator {
    void* allocate_batch(size_t count, size_t size) {
        // 一次分配所有内存
        void* block = sakura_malloc(count * size);
        
        // 返回连续内存块
        return block;
    }
};
```

## 总结

SakuraEngine 的内存管理系统提供了一套完整的内存管理解决方案。通过基于 mimalloc 的高性能分配器、灵活的智能指针系统、内存池支持以及完善的调试功能，系统能够满足游戏引擎对内存管理的各种需求。合理使用这些工具可以提高性能、避免内存泄漏，并简化内存管理的复杂性。