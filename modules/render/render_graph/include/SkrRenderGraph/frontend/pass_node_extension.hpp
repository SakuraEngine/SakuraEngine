#pragma once
#include "SkrRenderGraph/frontend/pass_node.hpp"

namespace skr {
namespace render_graph {

// Pass节点的Timeline扩展
class SKR_RENDER_GRAPH_API PassNodeTimelineExtension
{
public:
    // 异步计算提示 - 用于ComputePass
    virtual bool get_async_compute_hint() const { return false; }
    virtual void set_async_compute_hint(bool hint) {}
    
    // 独立执行提示 - 用于CopyPass
    virtual bool can_be_lone() const { return false; }
    virtual void set_can_be_lone(bool can) {}
    
    // 用户数据存储
    virtual void set_user_data(const char8_t* key, void* data) = 0;
    virtual void* get_user_data(const char8_t* key) const = 0;
};

// ComputePass的Timeline扩展
class SKR_RENDER_GRAPH_API ComputePassNode : public PassNode, public PassNodeTimelineExtension
{
public:
    ComputePassNode() : PassNode(EPassType::Compute) {}
    
    // 异步计算提示
    bool get_async_compute_hint() const override { return async_compute_hint_; }
    void set_async_compute_hint(bool hint) override { async_compute_hint_ = hint; }
    
    // 用户数据
    void set_user_data(const char8_t* key, void* data) override {
        user_data_[key] = data;
    }
    
    void* get_user_data(const char8_t* key) const override {
        auto it = user_data_.find(key);
        return it != user_data_.end() ? it->second : nullptr;
    }
    
private:
    bool async_compute_hint_ = false;
    mutable skr::HashMap<skr::String, void*> user_data_;
};

// CopyPass的Timeline扩展
class SKR_RENDER_GRAPH_API CopyPassNode : public PassNode, public PassNodeTimelineExtension
{
public:
    CopyPassNode() : PassNode(EPassType::Copy) {}
    
    // 独立执行提示
    bool can_be_lone() const override { return can_be_lone_; }
    void set_can_be_lone(bool can) override { can_be_lone_ = can; }
    
    // 用户数据
    void set_user_data(const char8_t* key, void* data) override {
        user_data_[key] = data;
    }
    
    void* get_user_data(const char8_t* key) const override {
        auto it = user_data_.find(key);
        return it != user_data_.end() ? it->second : nullptr;
    }
    
private:
    bool can_be_lone_ = false;
    mutable skr::HashMap<skr::String, void*> user_data_;
};

// 为现有的PassNode添加扩展方法
inline void enable_async_compute(ComputePassNode* pass)
{
    pass->set_async_compute_hint(true);
}

inline void enable_lone_copy(CopyPassNode* pass)
{
    pass->set_can_be_lone(true);
}

} // namespace render_graph
} // namespace skr