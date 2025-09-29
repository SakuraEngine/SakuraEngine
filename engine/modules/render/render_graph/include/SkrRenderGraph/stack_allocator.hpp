#pragma once
#include "SkrRenderGraph/api.h"
#include "SkrCore/memory/memory.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/map.hpp"
#include "SkrContainersDef/set.hpp"

namespace skr::RG
{

struct SKR_RENDER_GRAPH_API RenderGraphStackAllocator {
    struct CtorParam {  };
    
    static constexpr bool support_realloc = false; // Pool allocators don't support realloc
    
    RenderGraphStackAllocator(CtorParam param) noexcept;
    ~RenderGraphStackAllocator() noexcept;
    
    template <typename T>
    inline static T* alloc(size_t count)
    {
        return reinterpret_cast<T*>(alloc_raw(count, sizeof(T), alignof(T)));
    }
    
    template <typename T>
    inline static void free(T* p)
    {
        if (p) {
            free_raw(reinterpret_cast<void*>(p), alignof(T));
        }
    }
    
    template <typename T>
    inline static T* realloc(T* p, size_t size)
    {
        return nullptr;
    }
    
    // Raw allocation API
    static void* alloc_raw(size_t count, size_t item_size, size_t item_align);
    static void free_raw(void* p, size_t item_align);
    
    static void Initialize();
    static void Finalize();
    static void Reset();
    
    struct Impl;
};


template <typename T>
using StackVector = skr::Vector<T, RenderGraphStackAllocator>;

template <typename K, typename V>
using StackMap = skr::Map<K, V, skr::container::HashTraits<K>, RenderGraphStackAllocator>;

template <typename K>
using StackSet = skr::Set<K, skr::container::HashTraits<K>, RenderGraphStackAllocator>;

// STL兼容的Allocator模板
template <typename T>
class RenderGraphSTLAllocator
{
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    
    // 重绑定模板
    template <typename U>
    struct rebind
    {
        using other = RenderGraphSTLAllocator<U>;
    };
    
    // 构造函数
    RenderGraphSTLAllocator() noexcept = default;
    
    // 拷贝构造函数
    RenderGraphSTLAllocator(const RenderGraphSTLAllocator&) noexcept = default;
    
    // 不同类型的拷贝构造函数
    template <typename U>
    RenderGraphSTLAllocator(const RenderGraphSTLAllocator<U>&) noexcept {}
    
    // 析构函数
    ~RenderGraphSTLAllocator() = default;
    
    // 地址获取
    pointer address(reference x) const noexcept
    {
        return std::addressof(x);
    }
    
    const_pointer address(const_reference x) const noexcept
    {
        return std::addressof(x);
    }
    
    // 分配内存
    pointer allocate(size_type n, const void* = 0)
    {
        if (n == 0)
            return nullptr;
            
        void* p = RenderGraphStackAllocator::alloc_raw(n, sizeof(T), alignof(T));
        if (!p)
        {
            return nullptr;
        }
        return static_cast<pointer>(p);
    }
    
    // 释放内存
    void deallocate(pointer p, size_type n) noexcept
    {
        RenderGraphStackAllocator::free_raw(p, alignof(T));
    }
    
    // 最大分配数量
    size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
    
    // 构造对象
    template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }
    
    // 销毁对象
    template <typename U>
    void destroy(U* p)
    {
        p->~U();
    }
    
    // 比较运算符
    template <typename U>
    bool operator==(const RenderGraphSTLAllocator<U>&) const noexcept
    {
        return true; // 所有实例共享同一个后端
    }
    
    template <typename U>
    bool operator!=(const RenderGraphSTLAllocator<U>&) const noexcept
    {
        return false;
    }
};

template <typename K, typename V>
using StackHashMap = skr::FlatHashMap<K, V, 
    phmap::priv::hash_default_hash<K>, phmap::priv::hash_default_eq<K>, 
    RenderGraphSTLAllocator<phmap::priv::Pair<const K, V>>
>;

} // namespace skr::RG