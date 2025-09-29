#pragma once
#include "SkrBase/config.h"

namespace skr::RG {

// ==================== Hardware Performance Constants ====================

// 硬件约束常量（基于现代GPU硬件特性）
namespace HardwareConstraints {
    constexpr uint32_t MIN_COMMAND_BATCH_SIZE = 10;     // 硬件最小批处理大小
    constexpr uint32_t OPTIMAL_COMMAND_BATCH_SIZE = 20; // 最优批处理大小
    constexpr float CP_DISPATCH_COST_US = 5.0f;         // 命令处理器分发延迟（微秒）
    constexpr float OS_SCHEDULE_COST_US = 65.0f;        // 操作系统调度开销
}

// Barrier 成本（微秒）
namespace BarrierCosts {
    constexpr float SIMPLE_BARRIER = 2.5f;          // 简单资源屏障
    constexpr float L1_CACHE_FLUSH = 7.5f;          // L1 缓存刷新
    constexpr float L2_CACHE_FLUSH = 15.0f;         // L2 缓存刷新  
    constexpr float CROSS_QUEUE_SYNC = 35.0f;       // 跨队列同步
    constexpr float FORMAT_CONVERSION = 100.0f;     // 格式转换屏障
    constexpr float FULL_PIPELINE_FLUSH = 200.0f;   // 完整管线刷新
}

// 缓存层级（基于RDNA2/Ampere平均值）
namespace CacheHierarchy {
    constexpr size_t L0_SIZE_PER_CU = 16 * 1024;      // 每CU 16KB
    constexpr size_t L1_SIZE = 128 * 1024;            // 128KB
    constexpr size_t L2_SIZE = 6 * 1024 * 1024;       // 6MB
    constexpr size_t L3_SIZE = 96 * 1024 * 1024;      // 96MB（Infinity Cache和Ada L2平均）
    
    constexpr float L1_HIT_RATE_SEQUENTIAL = 0.55f;   // 顺序访问命中率
    constexpr float L1_HIT_RATE_RANDOM = 0.13f;       // 随机访问命中率
    constexpr float L2_HIT_RATE_TARGET = 0.95f;       // 目标L2命中率
}

// 异步计算约束
namespace AsyncConstraints {
    constexpr float MIN_ASYNC_WORKLOAD_MS = 0.5f;     // 异步计算最小工作负载
    constexpr uint32_t MAX_ASYNC_QUEUES = 32;         // 最大并发队列（AMD）
    constexpr float SYNC_OVERHEAD_US = 20.0f;         // 基础同步开销
}

// ==================== Core Data Structures ====================

// 硬件配置文件（运行时检测）
struct HardwareProfile {
    // 计算资源
    uint32_t num_compute_units = 32;
    uint32_t num_shader_engines = 2;
    uint32_t num_async_compute_engines = 4;
    
    // 内存层级
    size_t l1_cache_size = CacheHierarchy::L1_SIZE;
    size_t l2_cache_size = CacheHierarchy::L2_SIZE;
    size_t l3_cache_size = 0;  // 0表示不存在
    
    // 性能特征
    float memory_bandwidth_gbps = 500.0f;
    float barrier_base_cost_us = BarrierCosts::SIMPLE_BARRIER;
    float cp_dispatch_cost_us = HardwareConstraints::CP_DISPATCH_COST_US;
    
    // 厂商特定
    enum Vendor { NVIDIA, AMD, INTEL, UNKNOWN } vendor = UNKNOWN;
    bool supports_mesh_shaders = false;
    bool supports_ray_tracing = false;
};

} // namespace skr::RG