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

## 总结

SakuraEngine 的内存管理系统提供了一套完整的内存管理解决方案。通过基于 mimalloc 的高性能分配器、灵活的智能指针系统、内存池支持以及完善的调试功能，系统能够满足游戏引擎对内存管理的各种需求。合理使用这些工具可以提高性能、避免内存泄漏，并简化内存管理的复杂性。