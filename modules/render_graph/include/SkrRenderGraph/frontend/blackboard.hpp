#pragma once
#include "SkrRenderGraph/frontend/base_types.hpp"

namespace skr
{
namespace render_graph
{
struct Blackboard
{
    static Blackboard* Create() SKR_NOEXCEPT;
    static void Destroy(Blackboard* blackboard) SKR_NOEXCEPT;

    virtual void clear() SKR_NOEXCEPT = 0;
    virtual class PassNode* pass(const char* name) SKR_NOEXCEPT = 0;
    virtual class TextureNode* texture(const char* name) SKR_NOEXCEPT = 0;
    virtual class BufferNode* buffer(const char* name) SKR_NOEXCEPT = 0;

    virtual bool add_pass(const char* name, class PassNode* pass) SKR_NOEXCEPT = 0;
    virtual bool add_texture(const char* name, class TextureNode* texture) SKR_NOEXCEPT = 0;
    virtual bool add_buffer(const char* name, class BufferNode* buffer) SKR_NOEXCEPT = 0;

    virtual void override_pass(const char* name, class PassNode* pass) SKR_NOEXCEPT = 0;
    virtual void override_texture(const char* name, class TextureNode* texture) SKR_NOEXCEPT = 0;
    virtual void override_buffer(const char* name, class BufferNode* buffer) SKR_NOEXCEPT = 0;
};
} // namespace render_graph
} // namespace skr