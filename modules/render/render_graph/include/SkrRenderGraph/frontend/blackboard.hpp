#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
struct SKR_RENDER_GRAPH_API Blackboard
{
    static Blackboard* Create() SKR_NOEXCEPT;
    static void Destroy(Blackboard* blackboard) SKR_NOEXCEPT;
    virtual ~Blackboard() = default;

    virtual void clear() SKR_NOEXCEPT = 0;
    virtual class PassNode* pass(const char8_t* name) SKR_NOEXCEPT = 0;
    virtual class TextureNode* texture(const char8_t* name) SKR_NOEXCEPT = 0;
    virtual class BufferNode* buffer(const char8_t* name) SKR_NOEXCEPT = 0;
    virtual bool value(const char8_t* name, double& v) SKR_NOEXCEPT = 0;

    virtual bool add_pass(const char8_t* name, class PassNode* pass) SKR_NOEXCEPT = 0;
    virtual bool add_texture(const char8_t* name, class TextureNode* texture) SKR_NOEXCEPT = 0;
    virtual bool add_buffer(const char8_t* name, class BufferNode* buffer) SKR_NOEXCEPT = 0;
    virtual bool set_value(const char8_t* name, double v) SKR_NOEXCEPT = 0;

    virtual void override_pass(const char8_t* name, class PassNode* pass) SKR_NOEXCEPT = 0;
    virtual void override_texture(const char8_t* name, class TextureNode* texture) SKR_NOEXCEPT = 0;
    virtual void override_buffer(const char8_t* name, class BufferNode* buffer) SKR_NOEXCEPT = 0;
};
} // namespace render_graph
} // namespace skr