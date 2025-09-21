#pragma once
#include "SkrBase/math.h"
#include "SkrBase/atomic/atomic.h"
#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrContainersDef/bitset.hpp"
#include "SkrContainersDef/vector.hpp"

namespace skr {

/**
 * 支持动态resize的并发ID范围分配器
 * 使用分段设计减少竞争，支持扩容和缩容
 */
class IdRangeAllocator {
public:
    // ID 范围结构
    struct Range {
        uint32_t start;
        uint32_t length;
        
        Range(uint32_t s = 0, uint32_t l = 0) : start(s), length(l) {}
        uint32_t end() const { return start + length; }
        bool empty() const { return length == 0; }
        bool contains(uint32_t id) const { return id >= start && id < start + length; }
    };

private:
    static constexpr uint32_t IDS_PER_SEGMENT = 4096;    // 每段管理的ID数量
    static constexpr uint32_t BITMAP_SIZE = IDS_PER_SEGMENT; // bitset大小
    
    // ID 分配段
    struct Segment {
        SAtomicU32 next_id{0};                     // 下一个可分配的起始位置
        SAtomicU32 max_ids{0};                     // 这个段管理的最大ID数量
        skr::Bitset<BITMAP_SIZE, uint64_t> bitmap; // 位图，记录每个ID的分配状态
        
        Segment(uint32_t initial_max) : max_ids(initial_max) {
            bitmap.reset(); // 初始化为全0
        }
        
        // 扩容段，保留原有数据
        void expand(uint32_t new_max) {
            uint32_t old_max = skr_atomic_load_relaxed(&max_ids);
            if (new_max <= old_max) return;
            
            skr_atomic_store_relaxed(&max_ids, new_max);
        }
        
        // 缩容段，检查是否有已分配ID超出新范围
        bool shrink(uint32_t new_max) {
            uint32_t old_max = skr_atomic_load_relaxed(&max_ids);
            if (new_max >= old_max) return true;
            
            // 检查是否有超出新范围的已分配ID
            for (uint32_t id = new_max; id < old_max; ++id) {
                if (bitmap[id]) {
                    return false; // 不允许缩容，因为有已分配的ID超出新范围
                }
            }
            
            skr_atomic_store_relaxed(&max_ids, new_max);
            return true;
        }
        
        // 检查ID是否已分配
        bool isAllocated(uint32_t id) const {
            uint32_t max = skr_atomic_load_relaxed(&max_ids);
            if (id >= max) return false;
            return bitmap[id];
        }
        
        // 检查范围是否全部空闲
        bool isRangeFree(uint32_t start, uint32_t length) const {
            uint32_t max = skr_atomic_load_relaxed(&max_ids);
            if (start + length > max) return false;
            
            // 使用 bitset 的位操作优化
            for (uint32_t i = 0; i < length; ++i) {
                if (bitmap[start + i]) {
                    return false;
                }
            }
            return true;
        }
        
        // 标记范围状态
        void markRange(uint32_t start, uint32_t length, bool allocated) {
            for (uint32_t i = 0; i < length; ++i) {
                if (allocated) {
                    bitmap.set(start + i);
                } else {
                    bitmap.reset(start + i);
                }
            }
        }
    };
    
    Vector<Segment> segments_;                   // 段数组
    shared_atomic_mutex resize_mutex_;           // 保护resize操作
    SAtomicU32 max_ids_{0};                      // 当前最大ID数量
    SAtomicU32 total_allocated_{0};              // 已分配ID总数

public:
    IdRangeAllocator(uint32_t initial_max_ids = 65536) {
        resize(initial_max_ids);
    }
    
    ~IdRangeAllocator() = default;

    /**
     * 调整分配器大小（线程不安全）
     * @param new_max_ids 新的最大ID数量
     * @return true 如果调整成功，false 如果无法调整
     */
    bool resize(uint32_t new_max_ids) {
        resize_mutex_.lock();
        
        if (new_max_ids == skr_atomic_load_relaxed(&max_ids_)) {
            resize_mutex_.unlock();
            return true; // 大小相同，无需调整
        }
        
        uint32_t new_segment_count = (new_max_ids + IDS_PER_SEGMENT - 1) / IDS_PER_SEGMENT;
        uint32_t old_segment_count = segments_.size();
        
        // 调整段数量
        segments_.resize(new_segment_count, Segment(0));
        
        // 初始化或调整每个段
        for (uint32_t i = 0; i < new_segment_count; ++i) {
            uint32_t segment_max = skr::min(IDS_PER_SEGMENT, new_max_ids - i * IDS_PER_SEGMENT);
            
            if (i < old_segment_count) {
                // 现有段，尝试调整大小
                if (segment_max > skr_atomic_load_relaxed(&segments_[i].max_ids)) {
                    // 扩容
                    segments_[i].expand(segment_max);
                } else if (segment_max < skr_atomic_load_relaxed(&segments_[i].max_ids)) {
                    // 缩容
                    if (!segments_[i].shrink(segment_max)) {
                        resize_mutex_.unlock();
                        return false; // 缩容失败
                    }
                }
            } else {
                // 新段
                segments_[i] = Segment(segment_max);
            }
        }
        
        skr_atomic_store_relaxed(&max_ids_, new_max_ids);
        resize_mutex_.unlock();
        return true;
    }

    /**
     * 分配指定长度的连续ID段（线程安全）
     */
    Range allocate(uint32_t length) {
        if (length == 0 || length > IDS_PER_SEGMENT) {
            return Range{}; // 无效长度或超出单段限制
        }
        
        // 使用线程ID哈希选择起始段，分散访问
        std::hash<std::thread::id> hasher;
        uint32_t start_segment = hasher(std::this_thread::get_id()) % segments_.size();
        
        // 尝试从多个段分配
        for (uint32_t i = 0; i < segments_.size(); ++i) {
            uint32_t segment_id = (start_segment + i) % segments_.size();
            Range range = tryAllocateFromSegment(segment_id, length);
            if (!range.empty()) {
                return range;
            }
        }
        
        return Range{}; // 分配失败
    }

    /**
     * 释放ID范围（线程安全）
     */
    void deallocate(const Range& range) {
        if (range.empty()) return;
        
        uint32_t segment_id = range.start / IDS_PER_SEGMENT;
        if (segment_id >= segments_.size()) return;
        
        uint32_t local_start = range.start % IDS_PER_SEGMENT;
        Segment& segment = segments_[segment_id];
        
        segment.markRange(local_start, range.length, false);
        skr_atomic_fetch_sub_explicit(&total_allocated_, range.length, skr_memory_order_relaxed);
    }

    void deallocate(uint32_t id) {
        Range rng(id, 1);
        deallocate(rng);
    }

    /**
     * 检查ID是否已被分配（线程安全）
     */
    bool isAllocated(uint32_t id) const {
        uint32_t segment_id = id / IDS_PER_SEGMENT;
        if (segment_id >= segments_.size()) return false;
        
        const Segment& segment = segments_[segment_id];
        return segment.isAllocated(id % IDS_PER_SEGMENT);
    }

    /**
     * 获取当前最大ID数量
     */
    uint32_t getMaxIds() const {
        return skr_atomic_load_relaxed(&max_ids_);
    }

    /**
     * 获取统计信息
     */
    struct Stats {
        uint32_t max_ids;
        uint32_t allocated_count;
        uint32_t segment_count;
    };
    
    Stats getStats() const {
        return {
            skr_atomic_load_relaxed(&max_ids_),
            skr_atomic_load_relaxed(&total_allocated_),
            static_cast<uint32_t>(segments_.size())
        };
    }

    /**
     * 清空所有分配（线程不安全）
     */
    void clear() {
        resize_mutex_.lock();
        
        for (auto& segment : segments_) {
            segment.bitmap.reset();
            skr_atomic_store_relaxed(&segment.next_id, 0);
        }
        
        skr_atomic_store_relaxed(&total_allocated_, 0);
        resize_mutex_.unlock();
    }

private:
    /**
     * 尝试从指定段分配
     */
    Range tryAllocateFromSegment(uint32_t segment_id, uint32_t length) {
        if (segment_id >= segments_.size()) {
            return Range{};
        }
        
        Segment& segment = segments_[segment_id];
        uint32_t max_ids = skr_atomic_load_relaxed(&segment.max_ids);
        uint32_t start = skr_atomic_load_relaxed(&segment.next_id);
        
        // 尝试从当前位置开始查找
        for (uint32_t attempt = 0; attempt < max_ids; ++attempt) {
            uint32_t current_start = (start + attempt) % max_ids;
            
            if (segment.isRangeFree(current_start, length)) {
                // 使用CAS更新next_id
                uint32_t expected = start;
                uint32_t desired = (current_start + length) % max_ids;
                
                if (skr_atomic_compare_exchange_weak_explicit(&segment.next_id, &expected, desired, 
                    skr_memory_order_relaxed, skr_memory_order_relaxed)) {
                    
                    segment.markRange(current_start, length, true);
                    skr_atomic_fetch_add_explicit(&total_allocated_, length, skr_memory_order_relaxed);
                    
                    return Range{segment_id * IDS_PER_SEGMENT + current_start, length};
                }
            }
        }
        
        return Range{};
    }
};

} // namespace skr