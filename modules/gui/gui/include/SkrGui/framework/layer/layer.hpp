#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
// TODO. Layer
struct SKR_GUI_API Layer SKR_GUI_OBJECT_BASE {
    SKR_GUI_OBJECT_ROOT(Layer, "04589d18-7688-4b30-8239-c9c91ca6c9f7")
    SKR_GUI_RAII_MIX_IN()
    using VisitFuncRef = FunctionRef<void(NotNull<Layer*>)>;

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void         mount(NotNull<Layer*> parent) SKR_NOEXCEPT;
    void         unmount() SKR_NOEXCEPT;
    virtual void destroy() SKR_NOEXCEPT;
    virtual void attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT;
    virtual void detach() SKR_NOEXCEPT;
    virtual void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT = 0;

    // dirty
    void        mark_needs_composite() SKR_NOEXCEPT;
    inline bool needs_composite() const SKR_NOEXCEPT { return _needs_composite; }
    inline void cancel_needs_composite() SKR_NOEXCEPT { _needs_composite = false; }

    // getter
    inline Layer*         parent() const SKR_NOEXCEPT { return _parent; }
    inline PipelineOwner* owner() const SKR_NOEXCEPT { return _owner; }
    inline int32_t        depth() const SKR_NOEXCEPT { return _depth; }

private:
    // layer tree
    Layer*         _parent = nullptr;
    PipelineOwner* _owner  = nullptr;
    int32_t        _depth  = 0;

    // dirty
    bool _needs_composite = true;
};
} // namespace skr::gui