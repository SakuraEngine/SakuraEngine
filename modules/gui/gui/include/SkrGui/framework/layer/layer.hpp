#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/layer/layer.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "a40f7782-8b8f-4059-ab2a-3d8e4ac2fdd5",
    "rtti": true
)
SKR_GUI_API Layer : virtual public skr::rttr::IObject
{
    SKR_RTTR_GENERATE_BODY()
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
    // TODO. enable field reflection
    spush_attr("no-rtti": true)
    // layer tree
    Layer*         _parent = nullptr;
    PipelineOwner* _owner  = nullptr;
    int32_t        _depth  = 0;

    // dirty
    bool _needs_composite = true;
};
} // namespace gui sreflect
} // namespace skr sreflect