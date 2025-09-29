#include "SkrRenderGraph/frontend/pass_node.hpp"
#include "SkrRenderGraph/phases_v2/memory_aliasing_phase.hpp"
#include "SkrRenderGraph/phases_v2/resource_lifetime_analysis.hpp"
#include "SkrRenderGraph/phases_v2/cross_queue_sync_analysis.hpp"
#include "SkrRenderGraph/frontend/resource_node.hpp"
#include "SkrCore/log.hpp"
#include "SkrContainersDef/set.hpp"
#include <algorithm>

namespace skr {
namespace RG {

bool MemoryRegion::overlaps_with(const MemoryRegion& other) const
{
    if (!is_valid() || !other.is_valid()) return false;
    return !(offset + size <= other.offset || other.offset + other.size <= offset);
}

MemoryAliasingPhase::MemoryAliasingPhase(
    const PassInfoAnalysis& pass_info_analysis,
    const ResourceLifetimeAnalysis& lifetime_analysis,
    const CrossQueueSyncAnalysis& sync_analysis,
    const MemoryAliasingConfig& config)
    : config_(config)
    , pass_info_analysis_(pass_info_analysis)
    , lifetime_analysis_(lifetime_analysis)
    , sync_analysis_(sync_analysis)
{
}

void MemoryAliasingPhase::on_execute(RenderGraph* graph, RenderGraphFrameExecutor* executor, RenderGraphProfiler* profiler) SKR_NOEXCEPT
{
    SkrZoneScopedN("MemoryAliasingPhase");

    analyze_resources();
    calculate_aliasing_statistics();
    identify_aliasing_barriers();
    compute_alias_transitions();
    
    if (config_.enable_debug_output)
    {
        dump_aliasing_result();
        dump_memory_buckets();
    }
}

void MemoryAliasingPhase::analyze_resources() SKR_NOEXCEPT
{
    const auto& lifetime_result = lifetime_analysis_.get_result();
    
    // SSIS算法步骤1：按大小降序排序资源
    StackVector<ResourceNode*> sorted_resources = lifetime_result.resources_by_size_desc;
    
    // 过滤掉不需要内存管理的资源
    auto filtered_end = std::remove_if(sorted_resources.begin(), sorted_resources.end(),
        [this, &lifetime_result](ResourceNode* resource) {
            // 1. 跳过导入资源（外部管理内存）
            if (resource->is_imported())
            {
                if (config_.enable_debug_output)
                {
                    SKR_LOG_TRACE(u8"Skipping imported resource for aliasing: %s", resource->get_name());
                }
                return true;
            }
            
            // 2. 跳过独立执行的资源（特殊内存管理）
            if (resource->allow_lone())
            {
                if (config_.enable_debug_output)
                {
                    SKR_LOG_TRACE(u8"Skipping lone pass resource for aliasing: %s", resource->get_name());
                }
                return true;
            }
            
            // 3. 跳过太小的资源
            /*
            auto lifetime_it = lifetime_result.resource_lifetimes.find(resource);
            auto resource_info = pass_info_analysis_.get_resource_info(resource);
            if (!lifetime_it || resource_info->memory_size < config_.min_resource_size)
            {
                if (config_.enable_debug_output && lifetime_it)
                {
                    SKR_LOG_TRACE(u8"Skipping small resource for aliasing: %s (size: %llu bytes < %llu bytes)", 
                                 resource->get_name(), 
                                 resource_info->memory_size, 
                                 config_.min_resource_size);
                }
                return true;
            }
            */
            
            return false;
        });
    sorted_resources.resize_default(filtered_end - sorted_resources.begin());
    
    if (sorted_resources.is_empty())
    {
        // SKR_LOG_WARN(u8"MemoryAliasingPhase: No resources suitable for aliasing");
        return;
    }
    
    // SSIS算法步骤2：创建内存桶并尝试别名化
    perform_memory_aliasing(sorted_resources);
}

void MemoryAliasingPhase::perform_memory_aliasing(const StackVector<ResourceNode*>& sorted_resources) SKR_NOEXCEPT
{
    const bool useResourcePooling = config_.aliasing_tier == EAliasingTier::Tier0;
    const auto& lifetime_result = lifetime_analysis_.get_result();
    aliasing_result_.memory_buckets.reserve(4096);
    for (auto* resource : sorted_resources)
    {
        bool aliased = false;
        
        const bool use_pooling = get_aliasing_tier() == EAliasingTier::Tier0;
        const bool dynamic_resource = resource->get_tags() & kRenderGraphDynamicResourceTag;
        const bool can_aliasing = !use_pooling && !dynamic_resource;
        // 尝试放入现有桶中
        if (can_aliasing)
        {
            if (!config_.use_greedy_fit || useResourcePooling)
            {
                // First-Fit算法：使用第一个合适的桶
                for (auto& bucket : aliasing_result_.memory_buckets)
                {
                    if (try_alias_resource_in_bucket(resource, bucket))
                    {
                        aliased = true;
                        break;
                    }
                }
            }
            else
            {
                // 贪心算法：Best-Fit策略，选择浪费空间最少的桶
                MemoryBucket* best_bucket = nullptr;
                uint64_t best_bucket_waste = UINT64_MAX;
                
                for (auto& bucket : aliasing_result_.memory_buckets)
                {
                    // 检查能否放入这个桶，并计算浪费的空间
                    if (can_fit_in_bucket(resource, bucket))
                    {
                        uint64_t waste = calculate_bucket_waste(resource, bucket);
                        if (waste < best_bucket_waste)
                        {
                            best_bucket_waste = waste;
                            best_bucket = &bucket;
                        }
                    }
                }
                
                // 使用最优桶
                if (best_bucket && try_alias_resource_in_bucket(resource, *best_bucket))
                {
                    aliased = true;
                }
            }
        }
        
        // 如果无法放入现有桶，创建新桶
        if (!aliased && aliasing_result_.memory_buckets.size() < config_.max_buckets)
        {
            MemoryBucket new_bucket;
            auto lifetime_it = lifetime_result.resource_lifetimes.find(resource);
            auto resource_info = pass_info_analysis_.get_resource_info(resource);
            if (lifetime_it)
            {
                new_bucket.total_size = resource_info->memory_size;
                new_bucket.used_size = resource_info->memory_size;
                new_bucket.aliased_resources.add(resource);
                new_bucket.resource_offsets.add(resource, 0);
                new_bucket.original_total_size = resource_info->memory_size;
                
                uint32_t bucket_index = static_cast<uint32_t>(aliasing_result_.memory_buckets.size());
                aliasing_result_.memory_buckets.add(std::move(new_bucket));
                aliasing_result_.resource_to_bucket.add(resource, bucket_index);
                aliasing_result_.resource_to_offset.add(resource, 0);
                
                aliased = true;
            }
        }
        
        if (!aliased)
        {
            aliasing_result_.failed_to_alias_resources++;
            if (config_.enable_debug_output)
            {
                SKR_LOG_WARN(u8"Failed to alias resource: %s", resource->get_name());
            }
        }
        else
        {
            aliasing_result_.total_aliased_resources++;
        }
    }
}

bool MemoryAliasingPhase::try_alias_resource_in_bucket(ResourceNode* resource, MemoryBucket& bucket) SKR_NOEXCEPT
{
    const auto& lifetime_result = lifetime_analysis_.get_result();
    auto resource_info = pass_info_analysis_.get_resource_info(resource);
    auto resource_lifetime_it = lifetime_result.resource_lifetimes.find(resource);
    if (!resource_lifetime_it)
    {
        return false;
    }
    
    // 检查是否与桶中现有资源有时间冲突
    for (auto* existing_resource : bucket.aliased_resources)
    {
        if (!can_resources_alias(resource, existing_resource))
        {
            return false;
        }
    }

    MemoryRegion optimal_region = find_optimal_memory_region(resource, bucket);
    if (!optimal_region.is_valid())
    {
        return false; // 没有找到合适的区域
    }
    
    // 成功找到区域，更新桶信息
    bucket.aliased_resources.add(resource);
    bucket.resource_offsets.add(resource, optimal_region.offset);
    bucket.original_total_size += resource_info->memory_size;
    
    // 更新桶的总大小（如果新资源超出了当前范围）
    uint64_t resource_end = optimal_region.offset + resource_info->memory_size;
    if (resource_end > bucket.total_size)
    {
        bucket.total_size = resource_end;
    }
    
    // 更新全局映射
    uint32_t bucket_index = static_cast<uint32_t>(&bucket - &aliasing_result_.memory_buckets[0]);
    aliasing_result_.resource_to_bucket.add(resource, bucket_index);
    aliasing_result_.resource_to_offset.add(resource, optimal_region.offset);
    
    return true;
}

MemoryRegion MemoryAliasingPhase::find_optimal_memory_region(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT
{
    const auto& lifetime_result = lifetime_analysis_.get_result();
    auto resource_info = pass_info_analysis_.get_resource_info(resource);
    auto resource_lifetime_it = lifetime_result.resource_lifetimes.find(resource);
    if (!resource_lifetime_it)
    {
        return MemoryRegion{};
    }
    
    uint64_t resource_size = resource_info->memory_size;   
    // 如果桶为空，直接返回起始位置
    if (bucket.aliased_resources.is_empty())
    {
        return MemoryRegion{ 0, resource_size };
    }
    
    // 桶里有资源的情况
    const bool useResourcePooling = config_.aliasing_tier == EAliasingTier::Tier0;
    if (useResourcePooling)
    {
        // TODO：判断这个桶里的第一个资源是否与当前资源完全兼容
        return MemoryRegion( 0, 0 );
    }
    else
    {
        // SSIS算法：找到所有可别名的内存区域
        StackVector<MemoryRegion> aliasable_regions = find_aliasable_regions(resource, bucket);
        MemoryRegion optimal_region{};
        // 选择最小适配的区域（SSIS建议）
        for (const auto& region : aliasable_regions)
        {
            if (region.size >= resource_size)
            {
                if (!optimal_region.is_valid() || region.size < optimal_region.size)
                {
                    optimal_region = MemoryRegion{ region.offset, resource_size };
                }
            }
        }
        return optimal_region;
    }
}

StackVector<MemoryRegion> MemoryAliasingPhase::find_aliasable_regions(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT
{
    StackVector<MemoryRegion> aliasable_regions;
    
    // SSIS算法核心：收集非别名化的内存偏移
    StackVector<MemoryOffset> offsets;
    collect_non_aliasable_offsets(resource, bucket, offsets);
    
    if (offsets.is_empty())
    {
        // 没有冲突，整个桶都可用
        aliasable_regions.add(MemoryRegion{ 0, bucket.total_size });
        return aliasable_regions;
    }
    
    // 排序偏移点
    std::sort(offsets.begin(), offsets.end());
    
    // SSIS算法：使用重叠计数器找到自由区域
    int64_t overlap_counter = 0;
    
    for (size_t i = 0; i < offsets.size() - 1; ++i)
    {
        const auto& current_offset = offsets[i];
        const auto& next_offset = offsets[i + 1];
        
        // 更新重叠计数器
        overlap_counter += (current_offset.type == EMemoryOffsetType::Start) ? 1 : -1;
        overlap_counter = std::max(overlap_counter, 0ll);
        
        // 检查是否到达可别名区域
        bool reached_aliasable_region = 
            overlap_counter == 0 && 
            current_offset.type == EMemoryOffsetType::End && 
            next_offset.type == EMemoryOffsetType::Start;
        
        if (reached_aliasable_region)
        {
            uint64_t region_start = current_offset.offset;
            uint64_t region_size = next_offset.offset - current_offset.offset;
            
            if (region_size > 0)
            {
                aliasable_regions.add(MemoryRegion{ region_start, region_size });
            }
        }
    }
    
    return aliasable_regions;
}

void MemoryAliasingPhase::collect_non_aliasable_offsets(ResourceNode* resource, const MemoryBucket& bucket, 
                                                       StackVector<MemoryOffset>& offsets) const SKR_NOEXCEPT
{
    // 添加桶边界
    offsets.add(MemoryOffset{ 0, EMemoryOffsetType::End, nullptr });
    offsets.add(MemoryOffset{ bucket.total_size, EMemoryOffsetType::Start, nullptr });
    
    // 检查与桶中现有资源的冲突
    for (auto* existing_resource : bucket.aliased_resources)
    {
        if (resources_conflict_in_time(resource, existing_resource))
        {
            if (auto offset_it = bucket.resource_offsets.find(existing_resource))
            {
                const auto& lifetime_result = lifetime_analysis_.get_result();
                auto resource_info = pass_info_analysis_.get_resource_info(existing_resource);
                if (auto existing_lifetime_it = lifetime_result.resource_lifetimes.find(existing_resource))
                {
                    uint64_t start_offset = offset_it.value();
                    uint64_t end_offset = start_offset + resource_info->memory_size;
                    
                    offsets.add(MemoryOffset{ start_offset, EMemoryOffsetType::Start, existing_resource });
                    offsets.add(MemoryOffset{ end_offset, EMemoryOffsetType::End, existing_resource });
                }
            }
        }
    }
}

bool MemoryAliasingPhase::resources_conflict_in_time(ResourceNode* res1, ResourceNode* res2) const SKR_NOEXCEPT
{
    if (res1 == res2) return true;
    
    const auto& lifetime_result = lifetime_analysis_.get_result();
    auto lifetime1_it = lifetime_result.resource_lifetimes.find(res1);
    auto lifetime2_it = lifetime_result.resource_lifetimes.find(res2);
    
    if (!lifetime1_it || !lifetime2_it)
        return true;
    
    // 使用SSIS建议：dependency level indices进行冲突检测
    return lifetime1_it.value().conflicts_with(lifetime2_it.value());
}

bool MemoryAliasingPhase::can_resources_alias(ResourceNode* res1, ResourceNode* res2) const SKR_NOEXCEPT
{    
    // 检查生命周期冲突
    if (!resources_conflict_in_time(res1, res2))
    {
        // 如果不支持跨队列别名，检查队列兼容性
        if (!config_.enable_cross_queue_aliasing)
        {
            const auto& lifetime_result = lifetime_analysis_.get_result();
            auto lifetime1_it = lifetime_result.resource_lifetimes.find(res1);
            auto lifetime2_it = lifetime_result.resource_lifetimes.find(res2);
            
            if (lifetime1_it && lifetime2_it)
            {
                // 如果资源在不同队列使用，不允许别名
                if (lifetime1_it.value().primary_queue != lifetime2_it.value().primary_queue)
                {
                    if (config_.enable_debug_output)
                    {
                        SKR_LOG_TRACE(u8"Cannot alias cross-queue resources: %s (queue=%u) vs %s (queue=%u)",
                                     res1->get_name(), lifetime1_it.value().primary_queue,
                                     res2->get_name(), lifetime2_it.value().primary_queue);
                    }
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

void MemoryAliasingPhase::calculate_aliasing_statistics() SKR_NOEXCEPT
{
    aliasing_result_.total_original_memory = 0;
    aliasing_result_.total_aliased_memory = 0;
    
    for (const auto& bucket : aliasing_result_.memory_buckets)
    {
        aliasing_result_.total_original_memory += bucket.original_total_size;
        aliasing_result_.total_aliased_memory += bucket.total_size;
        
        // 计算每个桶的压缩比
        const_cast<MemoryBucket&>(bucket).compression_ratio = 
            1.0f - (static_cast<float>(bucket.total_size) / static_cast<float>(bucket.original_total_size));
    }
    
    // 计算总体压缩比
    if (aliasing_result_.total_original_memory > 0)
    {
        aliasing_result_.total_compression_ratio = 
            1.0f - (static_cast<float>(aliasing_result_.total_aliased_memory) / 
                   static_cast<float>(aliasing_result_.total_original_memory));
    }
}

void MemoryAliasingPhase::identify_aliasing_barriers() SKR_NOEXCEPT
{
    // 任何在同一桶中但有多个资源的情况都需要别名屏障
    for (const auto& bucket : aliasing_result_.memory_buckets)
    {
        if (bucket.aliased_resources.size() > 1)
        {
            // 所有在这个桶中的资源都需要别名屏障
            for (auto* resource : bucket.aliased_resources)
            {
                aliasing_result_.resources_need_aliasing_barrier.insert(resource);
            }
        }
    }
}

uint32_t MemoryAliasingPhase::get_resource_bucket(ResourceNode* resource) const
{
    auto it = aliasing_result_.resource_to_bucket.find(resource);
    return it ? it.value() : UINT32_MAX;
}

uint64_t MemoryAliasingPhase::get_resource_offset(ResourceNode* resource) const
{
    auto it = aliasing_result_.resource_to_offset.find(resource);
    return it ? it.value() : 0;
}

bool MemoryAliasingPhase::needs_aliasing_barrier(ResourceNode* resource) const
{
    return aliasing_result_.resources_need_aliasing_barrier.contains(resource);
}

uint64_t MemoryAliasingPhase::get_memory_savings() const
{
    return aliasing_result_.total_original_memory - aliasing_result_.total_aliased_memory;
}

void MemoryAliasingPhase::dump_aliasing_result() const SKR_NOEXCEPT
{
    SKR_LOG_INFO(u8"========== Memory Aliasing Results ==========");
    SKR_LOG_INFO(u8"Memory buckets: %zu", aliasing_result_.memory_buckets.size());
    SKR_LOG_INFO(u8"Aliased resources: %u", aliasing_result_.total_aliased_resources);
    SKR_LOG_INFO(u8"Failed to alias: %u", aliasing_result_.failed_to_alias_resources);
    SKR_LOG_INFO(u8"Original memory: %llu MB", aliasing_result_.total_original_memory / (1024 * 1024));
    SKR_LOG_INFO(u8"Aliased memory: %llu MB", aliasing_result_.total_aliased_memory / (1024 * 1024));
    SKR_LOG_INFO(u8"Memory savings: %llu MB ({:.1f}%)", 
                get_memory_savings() / (1024 * 1024),
                aliasing_result_.total_compression_ratio * 100.0f);
    SKR_LOG_INFO(u8"Resources needing barriers: %zu", aliasing_result_.resources_need_aliasing_barrier.size());
    SKR_LOG_INFO(u8"============================================");
}

void MemoryAliasingPhase::dump_memory_buckets() const SKR_NOEXCEPT
{
    SKR_LOG_INFO(u8"========== Memory Bucket Details ==========");
    
    for (size_t i = 0; i < aliasing_result_.memory_buckets.size(); ++i)
    {
        const auto& bucket = aliasing_result_.memory_buckets[i];
        
        SKR_LOG_INFO(u8"Bucket %zu:", i);
        SKR_LOG_INFO(u8"  Total size: %llu KB", bucket.total_size / 1024);
        SKR_LOG_INFO(u8"  Original size: %llu KB", bucket.original_total_size / 1024);
        SKR_LOG_INFO(u8"  Compression: %f%", bucket.compression_ratio * 100.0f);
        SKR_LOG_INFO(u8"  Resources (%zu):", bucket.aliased_resources.size());
        
        for (auto* resource : bucket.aliased_resources)
        {
            if (auto offset_it = bucket.resource_offsets.find(resource))
            {
                const auto& lifetime_result = lifetime_analysis_.get_result();
                auto resource_info = pass_info_analysis_.get_resource_info(resource);
                if (auto lifetime_it = lifetime_result.resource_lifetimes.find(resource))
                {
                    SKR_LOG_INFO(u8"    - %s: offset %llu, size %llu KB, levels [%u-%u]",
                                resource->get_name(),
                                offset_it.value(),
                                resource_info->memory_size / 1024,
                                lifetime_it.value().start_dependency_level,
                                lifetime_it.value().end_dependency_level);
                }
            }
        }
    }
    
    SKR_LOG_INFO(u8"==========================================");
}

void MemoryAliasingPhase::compute_alias_transitions() SKR_NOEXCEPT
{
    // 对每个内存桶，计算资源之间的转换点
    for (uint32_t bucket_idx = 0; bucket_idx < aliasing_result_.memory_buckets.size(); ++bucket_idx)
    {
        const auto& bucket = aliasing_result_.memory_buckets[bucket_idx];
        
        // 按资源的生命周期排序（开始时间）
        StackVector<ResourceNode*> sorted_resources = bucket.aliased_resources;
        sorted_resources.sort([this](ResourceNode* a, ResourceNode* b) {
                const auto& lifetime_result = lifetime_analysis_.get_result();
                auto a_lifetime = lifetime_result.resource_lifetimes.find(a);
                auto b_lifetime = lifetime_result.resource_lifetimes.find(b);
                
                if (a_lifetime && b_lifetime)
                {
                    return a_lifetime.value().start_dependency_level < b_lifetime.value().start_dependency_level;
                }
                return false;
            });
        
        // 计算资源之间的转换
        ResourceNode* current_resource = nullptr;
        for (auto* resource : sorted_resources)
        {
            record_alias_transition(nullptr, resource, bucket_idx);
            current_resource = resource;
        }
    }
    
    // 按执行顺序排序转换点
    std::sort(aliasing_result_.alias_transitions.begin(), aliasing_result_.alias_transitions.end(),
        [](const MemoryAliasTransition& a, const MemoryAliasTransition& b) {
            // 优先按目标资源的开始级别排序
            if (a.to_start_level != b.to_start_level)
                return a.to_start_level < b.to_start_level;
            // 如果相同，按源资源的结束级别排序
            return a.from_end_level < b.from_end_level;
        });
    
    aliasing_result_.total_alias_transitions = static_cast<uint32_t>(aliasing_result_.alias_transitions.size());
}

void MemoryAliasingPhase::record_alias_transition(ResourceNode* from, ResourceNode* to, uint32_t bucket_index) SKR_NOEXCEPT
{
    MemoryAliasTransition transition;
    transition.transition_pass = find_transition_pass(from, to);
    transition.from_resource = from;
    transition.to_resource = to;
    transition.bucket_index = bucket_index;
    
    // 获取资源的生命周期信息
    const auto& lifetime_result = lifetime_analysis_.get_result();
    
    if (from)
    {
        if (auto from_lifetime = lifetime_result.resource_lifetimes.find(from))
        {
            transition.source_pass = from_lifetime.value().last_using_pass;
            transition.from_end_level = from_lifetime.value().end_dependency_level;
            transition.before_state = from_lifetime.value().last_using_state;
        }
    }
    
    if (to)
    {
        auto resource_info = pass_info_analysis_.get_resource_info(to);
        if (auto to_lifetime = lifetime_result.resource_lifetimes.find(to))
        {
            transition.to_start_level = to_lifetime.value().start_dependency_level;
            transition.after_state = to_lifetime.value().first_using_state;

            transition.memory_size = resource_info->memory_size;
            transition.memory_offset = aliasing_result_.resource_to_offset.find(to).value();
        }
    }
    
    aliasing_result_.alias_transitions.add(transition);
    
    if (config_.enable_debug_output)
    {
        SKR_LOG_TRACE(u8"Alias transition: %s -> %s at pass %s (bucket %u)",
                     from ? from->get_name() : u8"<initial>",
                     to ? to->get_name() : u8"<none>",
                     transition.transition_pass ? transition.transition_pass->get_name() : u8"<unknown>",
                     bucket_index);
    }
}

PassNode* MemoryAliasingPhase::find_transition_pass(ResourceNode* from, ResourceNode* to) SKR_NOEXCEPT
{
    if (!to) return nullptr;
    
    // 转换发生在 'to' 资源的第一次使用时
    const auto& lifetime_result = lifetime_analysis_.get_result();
    auto resource_info = pass_info_analysis_.get_resource_info(to);
    auto to_lifetime = lifetime_result.resource_lifetimes.find(to);
    
    if (to_lifetime && !resource_info->used_states.is_empty())
    {
        return resource_info->used_states.at(0).key;
    }
    
    return nullptr;
}

bool MemoryAliasingPhase::can_fit_in_bucket(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT
{
    const auto& lifetime_result = lifetime_analysis_.get_result();
    auto resource_lifetime_it = lifetime_result.resource_lifetimes.find(resource);
    if (!resource_lifetime_it)
    {
        return false;
    }
    
    // 检查是否与桶中现有资源有时间冲突
    for (auto* existing_resource : bucket.aliased_resources)
    {
        if (!can_resources_alias(resource, existing_resource))
        {
            return false;
        }
    }
    
    // 检查是否能找到合适的内存区域
    MemoryRegion test_region = find_optimal_memory_region(resource, bucket);
    return test_region.is_valid();
}

uint64_t MemoryAliasingPhase::calculate_bucket_waste(ResourceNode* resource, const MemoryBucket& bucket) const SKR_NOEXCEPT
{
    const auto& lifetime_result = lifetime_analysis_.get_result();
    auto resource_info = pass_info_analysis_.get_resource_info(resource);
    auto resource_lifetime_it = lifetime_result.resource_lifetimes.find(resource);
    if (!resource_lifetime_it)
    {
        return UINT64_MAX; // 无法计算，返回最大值避免选择
    }
    
    // 找到最优放置位置
    MemoryRegion optimal_region = find_optimal_memory_region(resource, bucket);
    if (!optimal_region.is_valid())
    {
        return UINT64_MAX; // 无法放置，返回最大值
    }
    
    uint64_t resource_size = resource_info->memory_size;
    
    // 计算放入后桶的新大小
    uint64_t new_bucket_size = std::max(bucket.total_size, optimal_region.offset + resource_size);
    
    // 计算总的已使用内存（所有资源的原始大小）
    uint64_t total_used_memory = bucket.original_total_size + resource_size;
    
    // 浪费的内存 = 桶的总大小 - 实际使用的内存
    uint64_t waste = (new_bucket_size >= total_used_memory) ? 
                     (new_bucket_size - total_used_memory) : 0;
    
    return waste;
}

} // namespace RG
} // namespace skr