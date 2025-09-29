#include "SkrRenderGraph/stack_allocator.hpp"
#include "SkrContainersDef/stack_allocation.hpp"
#include "SkrOS/thread.h"
#include "SkrCore/log.hpp"
#include <memory>

namespace skr::RG
{

static const char* kStackAllocatorName = "RenderGraphStackAllocator";

// Implementation details
struct RenderGraphStackAllocator::Impl : public StackAllocator<RenderGraphStackAllocator::Impl>
{
    static constexpr size_t kDefaultChunkSize = 64 * 1024; // 64KB 默认块大小
    static constexpr size_t kAlignment = 16; // 16字节对齐
    static const char* GetAllocatorName() { return kStackAllocatorName; }
};

// Global singleton instance
static std::unique_ptr<RenderGraphStackAllocator::Impl> g_pool_impl = nullptr;
static SMutex g_instance_mutex = {};
static bool g_initialized = false;

void RenderGraphStackAllocator::Initialize() {
    if (g_initialized) return;
    
    skr_init_mutex(&g_instance_mutex);
    skr_mutex_acquire(&g_instance_mutex);
    {
        if (!g_initialized)
        {
            g_pool_impl = std::make_unique<Impl>();
            g_initialized = true;
            SKR_LOG_INFO(u8"RenderGraphStackAllocator initialized");
        }
    }
    skr_mutex_release(&g_instance_mutex);
}

void RenderGraphStackAllocator::Finalize() {
    if (!g_initialized) return;
    
    skr_mutex_acquire(&g_instance_mutex);
    {
        if (g_initialized)
        {
            g_pool_impl->finalize();
            g_pool_impl.reset();
            g_initialized = false;
            SKR_LOG_INFO(u8"RenderGraphStackAllocator finalized");
        }
    }
    skr_mutex_release(&g_instance_mutex);
    
    skr_destroy_mutex(&g_instance_mutex);
}

static RenderGraphStackAllocator::Impl& get_impl() {
    SKR_ASSERT(g_pool_impl && "RenderGraphStackAllocator not initialized! Call RenderGraphStackAllocator::Initialize() first.");
    return *g_pool_impl;
}

RenderGraphStackAllocator::RenderGraphStackAllocator(CtorParam param) noexcept 
{

}

RenderGraphStackAllocator::~RenderGraphStackAllocator() noexcept 
{

}

void* RenderGraphStackAllocator::alloc_raw(size_t count, size_t item_size, size_t item_align) 
{
    if (count == 0 || item_size == 0) return nullptr;
    
    size_t total_size = count * item_size;
    
    // 添加线程安全保护，与Reset()保持一致
    skr_mutex_acquire(&g_instance_mutex);
    void* result = get_impl().allocate(total_size, item_align);
    skr_mutex_release(&g_instance_mutex);
    
    return result;
}

void RenderGraphStackAllocator::free_raw(void* p, size_t item_align) 
{
    // Stack allocator忽略free操作
    // 内存将在reset()时被重用，在finalize()时被释放
}

// 添加reset和finalize的静态接口
void RenderGraphStackAllocator::Reset()
{
    if (g_initialized && g_pool_impl)
    {
        skr_mutex_acquire(&g_instance_mutex);
        g_pool_impl->reset();
        skr_mutex_release(&g_instance_mutex);
    }
}

} // namespace skr::RG