#pragma once
#include "SkrRenderGraph/frontend/render_graph.hpp"

namespace skr::RG
{

template<typename T>
class FrameResource
{
public:
    FrameResource() = default;
    ~FrameResource() = default;
    
    // 不可拷贝
    FrameResource(const FrameResource&) = delete;
    FrameResource& operator=(const FrameResource&) = delete;
    
    // 可移动
    FrameResource(FrameResource&&) = default;
    FrameResource& operator=(FrameResource&&) = default;
    
    // 获取当前帧的资源
    T& get(const RenderGraph* graph) SKR_NOEXCEPT
    {
        const uint64_t frame_index = graph->get_frame_index();
        const uint32_t index = static_cast<uint32_t>(frame_index % max_frames_in_flight());
        return resources[index];
    }
    
    const T& get(const RenderGraph* graph) const SKR_NOEXCEPT
    {
        const uint64_t frame_index = graph->get_frame_index();
        const uint32_t index = static_cast<uint32_t>(frame_index % max_frames_in_flight());
        return resources[index];
    }
    
    // 获取指定帧偏移的资源 (0 = 当前帧, 1 = 上一帧, etc.)
    T& get_frame_offset(const RenderGraph* graph, uint32_t frame_offset) SKR_NOEXCEPT
    {
        const uint64_t frame_index = graph->get_frame_index();
        const uint32_t index = static_cast<uint32_t>((frame_index + max_frames_in_flight() - frame_offset) % max_frames_in_flight());
        return resources[index];
    }
    
    const T& get_frame_offset(const RenderGraph* graph, uint32_t frame_offset) const SKR_NOEXCEPT
    {
        const uint64_t frame_index = graph->get_frame_index();
        const uint32_t index = static_cast<uint32_t>((frame_index + max_frames_in_flight() - frame_offset) % max_frames_in_flight());
        return resources[index];
    }
    
    // 直接通过索引访问（用于初始化等场景）
    T& operator[](uint32_t index) SKR_NOEXCEPT
    {
        SKR_ASSERT(index < max_frames_in_flight());
        return resources[index];
    }
    
    const T& operator[](uint32_t index) const SKR_NOEXCEPT
    {
        SKR_ASSERT(index < max_frames_in_flight());
        return resources[index];
    }
    
    // 获取最大帧数（额外增加一帧用于异步重叠处理）
    static constexpr uint32_t max_frames_in_flight() SKR_NOEXCEPT
    {
        return RG_MAX_FRAME_IN_FLIGHT;
    }
    
    // 迭代器支持
    auto begin() SKR_NOEXCEPT { return resources.begin(); }
    auto end() SKR_NOEXCEPT { return resources.end(); }
    auto begin() const SKR_NOEXCEPT { return resources.begin(); }
    auto end() const SKR_NOEXCEPT { return resources.end(); }

private:
    T resources[RG_MAX_FRAME_IN_FLIGHT];
};

} // namespace skr::RG