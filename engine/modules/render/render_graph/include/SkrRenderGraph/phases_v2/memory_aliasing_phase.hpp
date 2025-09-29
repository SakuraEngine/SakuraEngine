#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrRenderGraph/frontend/base_types.hpp"
#include "resource_lifetime_analysis.hpp"
#include "cross_queue_sync_analysis.hpp"
#include "SkrContainersDef/vector.hpp"
#include "SkrContainersDef/map.hpp"

namespace skr {
namespace RG {

// 内存区域描述（基于SSIS算法）
struct MemoryRegion
{
    uint64_t offset = 0;        // 内存偏移
    uint64_t size = 0;          // 区域大小
    
    bool is_valid() const { return size > 0; }
    bool contains(uint64_t pos) const { return pos >= offset && pos < (offset + size); }
    bool overlaps_with(const MemoryRegion& other) const;
};

// 内存偏移类型（SSIS算法核心）
enum class EMemoryOffsetType : uint8_t
{
    Start,  // 资源开始位置
    End     // 资源结束位置
};

// 内存偏移点（用于SSIS的重叠计数算法）
struct MemoryOffset
{
    uint64_t offset;
    EMemoryOffsetType type;
    ResourceNode* resource;     // 关联的资源
    
    bool operator<(const MemoryOffset& other) const { return offset < other.offset; }
};

// 内存桶（Memory Bucket）- SSIS算法的核心概念
struct MemoryBucket
{
    uint64_t total_size = 0;                        // 桶的总大小
    uint64_t used_size = 0;                         // 已使用大小
    StackVector<ResourceNode*> aliased_resources;   // 别名化的资源列表
    StackMap<ResourceNode*, uint64_t> resource_offsets; // 资源在桶中的偏移
    
    // 统计信息
    uint64_t original_total_size = 0;               // 不别名化时的总大小
    float compression_ratio = 0.0f;                 // 压缩比
};

// 内存别名转换点 - 记录内存从一个资源切换到另一个资源的时刻
struct MemoryAliasTransition
{
    PassNode* source_pass = nullptr;           // 源Pass
    PassNode* transition_pass = nullptr;       // 发生转换的Pass
    ResourceNode* from_resource = nullptr;     // 前一个资源（可能为空，表示首次使用）
    ResourceNode* to_resource = nullptr;       // 新资源
    ECGPUResourceState before_state = CGPU_RESOURCE_STATE_UNDEFINED; // 前一个资源的状态
    ECGPUResourceState after_state = CGPU_RESOURCE_STATE_UNDEFINED;   // 新资源

    uint32_t bucket_index = UINT32_MAX;        // 内存桶索引
    uint64_t memory_offset = 0;                // 在桶中的偏移
    uint64_t memory_size = 0;                  // 资源大小
    
    uint32_t from_end_level = 0;               // from_resource的结束依赖级别
    uint32_t to_start_level = 0;               // to_resource的开始依赖级别
};

// 内存别名分析结果
struct MemoryAliasingResult
{
    // 内存桶列表
    StackVector<MemoryBucket> memory_buckets;
    
    // 资源到桶的映射
    StackMap<ResourceNode*, uint32_t> resource_to_bucket;
    StackMap<ResourceNode*, uint64_t> resource_to_offset;
    
    // 内存别名转换点列表
    StackVector<MemoryAliasTransition> alias_transitions;
    
    // 需要别名屏障的资源
    skr::FlatHashSet<ResourceNode*> resources_need_aliasing_barrier;
    
    // 统计信息
    uint64_t total_original_memory = 0;     // 原始内存需求
    uint64_t total_aliased_memory = 0;      // 别名化后内存需求
    float total_compression_ratio = 0.0f;   // 总体压缩比
    uint32_t total_aliased_resources = 0;   // 成功别名化的资源数
    uint32_t failed_to_alias_resources = 0; // 失败的资源数
    uint32_t total_alias_transitions = 0;   // 别名转换次数
};

enum class EAliasingTier
{
    Tier0, // 无别名化, 资源池
    Tier1  // 有线性堆别名
};

// 内存别名配置
struct MemoryAliasingConfig
{
    EAliasingTier aliasing_tier = EAliasingTier::Tier0; // 别名化级别，默认使用真正的别名
    bool enable_cross_queue_aliasing = true;  // 允许跨队列资源别名
    uint64_t min_resource_size = 1024;        // 最小别名资源大小
    uint32_t max_buckets = UINT32_MAX;                // 最大桶数量
    bool enable_debug_output = false;         // 启用调试输出
    bool use_greedy_fit = true;               // 使用贪婪适配算法
};

// 内存别名Phase
class SKR_RENDER_GRAPH_API MemoryAliasingPhase : public IRenderGraphPhase
{
public:
    MemoryAliasingPhase(
        const PassInfoAnalysis& pass_info_analysis,
        const ResourceLifetimeAnalysis& lifetime_analysis,
        const CrossQueueSyncAnalysis& sync_analysis,
        const MemoryAliasingConfig& config = {});
    ~MemoryAliasingPhase() override = default;

    // IRenderGraphPhase 接口
    void on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT override;

    // 查询接口
    const MemoryAliasingResult& get_result() const { return aliasing_result_; }
    uint32_t get_resource_bucket(ResourceNode* resource) const;
    uint64_t get_resource_offset(ResourceNode* resource) const;
    bool needs_aliasing_barrier(ResourceNode* resource) const;
    const ResourceLifetimeAnalysis& get_lifetime_analysis() const { return lifetime_analysis_; }
    EAliasingTier get_aliasing_tier() const { return config_.aliasing_tier; }

    // 内存优化查询
    float get_compression_ratio() const { return aliasing_result_.total_compression_ratio; }
    uint64_t get_memory_savings() const;
    
    // 调试接口
    void dump_aliasing_result() const SKR_NOEXCEPT;
    void dump_memory_buckets() const SKR_NOEXCEPT;

private:
    // 核心算法（基于SSIS实现）
    void analyze_resources() SKR_NOEXCEPT;
    void create_memory_buckets() SKR_NOEXCEPT;
    void perform_memory_aliasing(const StackVector<ResourceNode*>&) SKR_NOEXCEPT;

    bool try_alias_resource_in_bucket(ResourceNode* resource, MemoryBucket& bucket) SKR_NOEXCEPT;
    MemoryRegion find_optimal_memory_region(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT;
    
    // SSIS算法核心：找到可别名的内存区域
    StackVector<MemoryRegion> find_aliasable_regions(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT;
    void collect_non_aliasable_offsets(ResourceNode* resource, const MemoryBucket& bucket, 
                                     StackVector<MemoryOffset>& offsets) const SKR_NOEXCEPT;
    
    // 别名转换点计算
    void compute_alias_transitions() SKR_NOEXCEPT;
    void record_alias_transition(ResourceNode* from, ResourceNode* to, uint32_t bucket_index) SKR_NOEXCEPT;
    PassNode* find_transition_pass(ResourceNode* from, ResourceNode* to) SKR_NOEXCEPT;
    
    // 辅助方法
    bool resources_conflict_in_time(ResourceNode* res1, ResourceNode* res2) const SKR_NOEXCEPT;
    bool can_resources_alias(ResourceNode* res1, ResourceNode* res2) const SKR_NOEXCEPT;
    void calculate_aliasing_statistics() SKR_NOEXCEPT;
    void identify_aliasing_barriers() SKR_NOEXCEPT;
    
    // 贪心算法辅助方法
    bool can_fit_in_bucket(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT;
    uint64_t calculate_bucket_waste(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT;

private:
    // 配置
    MemoryAliasingConfig config_;

    // 输入Phase引用
    const PassInfoAnalysis& pass_info_analysis_;
    const ResourceLifetimeAnalysis& lifetime_analysis_;
    const CrossQueueSyncAnalysis& sync_analysis_;

    // 分析结果
    MemoryAliasingResult aliasing_result_;
};

} // namespace RG
} // namespace skr