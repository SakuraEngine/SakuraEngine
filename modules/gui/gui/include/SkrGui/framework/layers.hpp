#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "177d0afa-6efe-4205-b13b-ed67e1d31292"
)
SKR_GUI_API Layer SKR_GUI_OBJECT_BASE {
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

sreflect_struct(
    "guid": "07997d78-3663-44c0-8944-c5fc3482e126"
)
GeometryLayer : public Layer {
    SKR_GUI_OBJECT(GeometryLayer, "8594695b-da35-462a-b4bf-b4ab891088c5", Layer)
    using Super = Layer;

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT override;
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    inline ICanvas* canvas() const SKR_NOEXCEPT { return _canvas; }

private:
    ICanvas* _canvas = nullptr;
};

sreflect_struct(
    "guid": "92d422cf-e21b-4698-bca3-80e99e5e613e"
)
SKR_GUI_API ContainerLayer : public Layer {
    SKR_GUI_OBJECT(ContainerLayer, "bb5a6b46-30b2-49f3-b445-260f9372bbb5", Layer)

    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override;

    void add_child(NotNull<Layer*> child) SKR_NOEXCEPT;
    bool has_children() const SKR_NOEXCEPT;
    void remove_all_children() SKR_NOEXCEPT;

    inline Span<Layer* const> children() const SKR_NOEXCEPT { return { _children.data(), _children.size() }; }

private:
    Array<Layer*> _children;
};

sreflect_struct(
    "guid": "759c6ded-2024-4106-9e16-42e4ea9c6b97"
)
SKR_GUI_API OffsetLayer : public ContainerLayer {
    SKR_GUI_OBJECT(OffsetLayer, "739c87d9-0e84-4300-a15e-cf892ca8379f", ContainerLayer)

    inline void    set_offset(Offsetf offset) noexcept { _offset = offset; }
    inline Offsetf offset() const noexcept { return _offset; }

private:
    Offsetf _offset = {};
};

sreflect_struct(
    "guid": "600a6782-2484-42e8-8a91-3defe4862d75"
)
SKR_GUI_API WindowLayer : public OffsetLayer {
    SKR_GUI_OBJECT(WindowLayer, "7ffdd5d0-9ed8-4f1b-87ab-8e08a29333c2", OffsetLayer)
    WindowLayer(IWindow* window);

    void update_window();

private:
    IWindow* _window = nullptr;
};

sreflect_struct(
    "guid": "3f09d1ea-8152-4018-9486-10d41d881add"
)
SKR_GUI_API NativeWindowLayer : public WindowLayer {
    SKR_GUI_OBJECT(NativeWindowLayer, "e787005d-8633-42a0-87f3-841e1b9435b3", WindowLayer)
    NativeWindowLayer(INativeWindow* native_window);
};

}
} // namespace skr sreflect