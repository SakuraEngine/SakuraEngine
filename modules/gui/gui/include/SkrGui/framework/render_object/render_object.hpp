#pragma once
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/matrix.hpp"
#include "SkrGui/framework/slot.hpp"
#include "SkrGui/system/input/hit_test.hpp"
#ifndef __meta__
    #include "SkrGui/framework/render_object/render_object.generated.h"
#endif

namespace skr::gui
{

enum class ERenderObjectLifecycle : uint8_t
{
    Initial,
    Mounted,
    Unmounted,
    Destroyed,
};

sreflect_struct(
    "guid" : "2f1b78a5-1be9-4799-a3ca-2f2d3b153f29"
)
SKR_GUI_API RenderObject : virtual public skr::rttr::IObject {
    SKR_GENERATE_BODY()
    friend struct BuildOwner;
    friend struct PaintingContext;
    using VisitFuncRef = FunctionRef<bool(NotNull<RenderObject*>)>;

    RenderObject() SKR_NOEXCEPT;
    virtual ~RenderObject();

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void                          mount(NotNull<RenderObject*> parent) SKR_NOEXCEPT; // 挂到父节点下
    void                          unmount() SKR_NOEXCEPT;                            // 从父节点卸载
    virtual void                  destroy() SKR_NOEXCEPT;                            // 销毁，生命周期结束
    virtual void                  attach(NotNull<BuildOwner*> owner) SKR_NOEXCEPT;   // 自身或子树挂载到带有 pipeline owner 的树下
    virtual void                  detach() SKR_NOEXCEPT;                             // 自身或子树从带有 pipeline owner 的树下卸载
    inline ERenderObjectLifecycle lifecycle() const SKR_NOEXCEPT { return _lifecycle; }
    inline BuildOwner*            owner() const SKR_NOEXCEPT { return _owner; }
    virtual void                  visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT = 0;

    // layout & paint marks
    virtual void mark_needs_layout() SKR_NOEXCEPT;
    virtual void mark_needs_paint() SKR_NOEXCEPT;
    void         mark_needs_layer_update() SKR_NOEXCEPT;
    inline bool  needs_layout() const SKR_NOEXCEPT { return _needs_layout; }
    inline bool  needs_paint() const SKR_NOEXCEPT { return _needs_paint; }
    inline bool  needs_layer_update() const SKR_NOEXCEPT { return _needs_layer_update; }
    inline void  cancel_needs_layout() SKR_NOEXCEPT { _needs_layout = false; }
    inline void  cancel_needs_paint() SKR_NOEXCEPT { _needs_paint = false; }
    inline void  cancel_needs_layer_update() SKR_NOEXCEPT { _needs_layer_update = false; }

    // layout process
    // 1. 传递 constraints 并标记 _is_constraints_changed
    // 2. 调用 layout()，并解析 layout_boundary 信息
    //    2.1 [if !_needs_layout && !_is_constraints_changed] 向下传递 layout_boundary 信息
    //    2.2 [if is_sized_by_parent()] 说明 child 尺寸计算完全由传递的约束决定，调用 perform_resize()
    //    2.3 调用 perform_layout()
    // 3. 清除 _is_relayout_boundary 与 _is_constraints_changed 标记
    // Note: parent_uses_size 主要作用是影响 child 重新布局向父亲的信息传播
    virtual bool is_sized_by_parent() const SKR_NOEXCEPT;
    void         layout(bool parent_uses_size = false) SKR_NOEXCEPT;
    virtual void perform_resize() SKR_NOEXCEPT;
    virtual void perform_layout() SKR_NOEXCEPT;

    // paint process
    // paint 流程由 layer 发起，其调用流程被 PaintingContext 严格封装，不允许直接调用，而是调用 PaintingContext::paintChild
    virtual void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT; // TODO. perform_paint
    virtual bool is_repaint_boundary() const SKR_NOEXCEPT;
    inline bool  was_repaint_boundary() const SKR_NOEXCEPT { return _was_repaint_boundary; }

    // layer composite
    // repaint boundary render object 会持有 layer 来实现局部重绘
    // 部分 repaint boundary render object 会通过 layer 来实现特效，比如毛玻璃，局部透明，复杂蒙版等等
    virtual NotNull<OffsetLayer*> update_layer(OffsetLayer* old_layer);
    inline ContainerLayer*        layer() const SKR_NOEXCEPT { return _layer; }

    // transform
    // 用于做坐标点转换，通常用于 hit-test
    virtual bool paints_child(NotNull<const RenderObject*> child) const SKR_NOEXCEPT; // 检测该 child 是否真的会发生 paint
    virtual void apply_paint_transform(NotNull<const RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT;
    Matrix4      get_transform_to(const RenderObject* ancestor = nullptr) const SKR_NOEXCEPT;
    Offsetf      global_to_local(Offsetf global_position, const RenderObject* ancestor = nullptr) const SKR_NOEXCEPT;
    Offsetf      local_to_global(Offsetf local_position, const RenderObject* ancestor = nullptr) const SKR_NOEXCEPT;
    Offsetf      system_to_local(Offsetf system_position) const SKR_NOEXCEPT;
    Offsetf      local_to_system(Offsetf local_position) const SKR_NOEXCEPT;

    // event
    virtual bool handle_event(NotNull<PointerEvent*> event, NotNull<HitTestEntry*> entry);

    // TODO
    // invoke_layout_callback：用于在 layout 过程中创建 child，通常用于 Sliver
    // show_on_screen：或许可以实现，用于 ScrollView 的目标追踪

    // getter
    inline RenderObject* parent() const SKR_NOEXCEPT { return _parent; }
    inline int32_t       depth() const SKR_NOEXCEPT { return _depth; }
    inline Slot          slot() const SKR_NOEXCEPT { return _slot; }

    // setter
    inline void set_slot(Slot slot) SKR_NOEXCEPT { _slot = slot; }
    inline void set_layer(ContainerLayer* layer) SKR_NOEXCEPT { _layer = layer; }

protected:
    void        _mark_parent_needs_layout() SKR_NOEXCEPT;
    inline void _set_force_relayout_boundary(bool v) SKR_NOEXCEPT { _force_relayout_boundary = v; }
    inline void _set_constraints_changed(bool v) SKR_NOEXCEPT { _is_constraints_changed = v; }
    inline void _update_was_repaint_boundary() SKR_NOEXCEPT { _was_repaint_boundary = is_repaint_boundary(); }

    // for special mount progress
    friend struct RenderNativeWindow;

private:
    void _flush_relayout_boundary() SKR_NOEXCEPT;

private:
    // render object tree
    RenderObject* _parent = nullptr;
    BuildOwner*   _owner  = nullptr;
    int32_t       _depth  = 0;

    // dirty marks
    bool _needs_layout       = true;
    bool _needs_paint        = true;
    bool _needs_layer_update = true;
    // TODO. compositing layer & compositing bits，其中 Bits 是 Flutter Picture 位图化的产物，我们或许只需要标记 Layer 的更新

    // layout temporal data
    bool _force_relayout_boundary = false; // 强制自己称为 layout_boundary
    bool _is_constraints_changed  = false; // 约束发生变化，在 layout 结束后被清理

    // layout & paint boundary
    bool            _was_repaint_boundary = false;
    RenderObject*   _relayout_boundary    = nullptr;
    ContainerLayer* _layer                = nullptr;

    // 用于 invoke_layout_callback()
    bool _doing_this_layout_with_callback = false;

    // 生命周期
    ERenderObjectLifecycle _lifecycle = ERenderObjectLifecycle::Initial;

    // parent 分配的实际 Slot 位置
    Slot _slot = {};
};

} // namespace skr::gui