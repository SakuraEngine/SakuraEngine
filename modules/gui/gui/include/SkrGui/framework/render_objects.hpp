#pragma once
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/math/geometry.hpp"
#include "SkrGui/math/matrix.hpp"
#include "SkrGui/math/layout.hpp"
#include "SkrGui/framework/slot.hpp"

/**********************************feature interfaces**********************************/
namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid": "4e5c6972-b829-470f-9104-a81567e47e29"
)
SKR_GUI_API IMultiChildRenderObject SKR_GUI_INTERFACE_BASE {
    virtual ~IMultiChildRenderObject() = default;

    virtual SKR_GUI_TYPE_ID accept_child_type() const SKR_NOEXCEPT                                    = 0;
    virtual void            add_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT           = 0;
    virtual void            remove_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT        = 0;
    virtual void            move_child(NotNull<RenderObject*> child, Slot from, Slot to) SKR_NOEXCEPT = 0;
    virtual void            flush_updates() SKR_NOEXCEPT                                              = 0;
};

template <typename TChild, typename TSlotData>
struct SlotStorage {
    // slot data
    TSlotData data = {};

    // child data
    Slot    desired_slot = {}; // used for slot update
    TChild* child        = nullptr;

    inline SlotStorage() SKR_NOEXCEPT = default;
    inline SlotStorage(Slot slot, TChild* child) SKR_NOEXCEPT : desired_slot(slot), child(child) {}
};
template <typename TChild>
struct SlotStorage<TChild, void> {
    Slot    desired_slot = {}; // used for slot update
    TChild* child        = nullptr;

    inline SlotStorage() SKR_NOEXCEPT = default;
    inline SlotStorage(Slot slot, TChild* child) SKR_NOEXCEPT : desired_slot(slot), child(child) {}
};
template <typename TSelf, typename TChild, typename TSlotData>
struct MultiChildRenderObjectMixin {
    Array<SlotStorage<TChild, TSlotData>> _children;
    bool                                  _need_flush_updates = false;

    inline SKR_GUI_TYPE_ID accept_child_type(const TSelf& self) const SKR_NOEXCEPT
    {
        return SKR_GUI_TYPE_ID_OF_STATIC(TChild);
    }
    inline void add_child(TSelf& self, NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
    {
        _children.emplace_back(slot, child->type_cast_fast<TChild>());
        _need_flush_updates = true;
    }
    inline void remove_child(TSelf& self, NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT
    {
        auto& child_slot = _children[child->slot().index];
        if (child_slot.desired_slot != slot) { SKR_GUI_LOG_ERROR(u8"slot miss match when remove child"); }
        child_slot.child->unmount();
        child_slot.child    = nullptr;
        _need_flush_updates = true;
    }
    inline void move_child(TSelf& self, NotNull<RenderObject*> child, Slot from, Slot to) SKR_NOEXCEPT
    {
        if (from != _children[child->slot().index].desired_slot) SKR_GUI_LOG_ERROR(u8"slot miss match when move child");
        _children[child->slot().index].desired_slot = to;
        _need_flush_updates                         = true;
    }
    inline void flush_updates(TSelf& self) SKR_NOEXCEPT
    {
        if (_need_flush_updates)
        {
            // step 1: remove all null slot
            for (size_t i = 0; i < _children.size();)
            {
                if (!_children[i].child)
                {
                    _remove_at_swap(i);
                }
                else
                {
                    ++i;
                }
            }

            // step 2: resort slot
            _sort_slots();

            // step 3: assign and check slot index
            for (size_t i = 0; i < _children.size(); ++i)
            {
                const auto& slot = _children[i];
                if (slot.desired_slot.index != i) SKR_GUI_LOG_ERROR(u8"slot index miss match");
                slot.child->set_slot(slot.desired_slot);
            }
        }

        _need_flush_updates = false;
    }

    inline void _remove_at_swap(size_t pos)
    {
        _children[pos] = _children.back();
        _children.pop_back();
    }
    inline void _sort_slots()
    {
        std::sort(_children.begin(), _children.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.desired_slot.index < rhs.desired_slot.index;
        });
    }
    inline void visit_children(const TSelf& self, RenderObject::VisitFuncRef visitor) const SKR_NOEXCEPT
    {
        for (const auto& slot : _children)
        {
            if (slot.child)
            {
                visitor(make_not_null(slot.child));
            }
        }
    }
};

#define MULTI_CHILD_RENDER_OBJECT_MIX_IN(__SELF, __CHILD, __SLOT_DATA)                                \
    /*===============> Begin Multi Child Render Object Mixin <===============*/                       \
private:                                                                                              \
    MultiChildRenderObjectMixin<__SELF, __CHILD, __SLOT_DATA> _multi_child_render_object_mix_in = {}; \
                                                                                                      \
public:                                                                                               \
    SKR_GUI_TYPE_ID accept_child_type() const SKR_NOEXCEPT override                                   \
    {                                                                                                 \
        return _multi_child_render_object_mix_in.accept_child_type(*this);                            \
    }                                                                                                 \
    void add_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override                     \
    {                                                                                                 \
        _multi_child_render_object_mix_in.add_child(*this, child, slot);                              \
    }                                                                                                 \
    void remove_child(NotNull<RenderObject*> child, Slot slot) SKR_NOEXCEPT override                  \
    {                                                                                                 \
        _multi_child_render_object_mix_in.remove_child(*this, child, slot);                           \
    }                                                                                                 \
    void move_child(NotNull<RenderObject*> child, Slot from, Slot to) SKR_NOEXCEPT override           \
    {                                                                                                 \
        _multi_child_render_object_mix_in.move_child(*this, child, from, to);                         \
    }                                                                                                 \
    void flush_updates() SKR_NOEXCEPT override                                                        \
    {                                                                                                 \
        _multi_child_render_object_mix_in.flush_updates(*this);                                       \
    }                                                                                                 \
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override                             \
    {                                                                                                 \
        _multi_child_render_object_mix_in.visit_children(*this, visitor);                             \
    }                                                                                                 \
    inline const Array<SlotStorage<__CHILD, __SLOT_DATA>>& children() const SKR_NOEXCEPT              \
    {                                                                                                 \
        return _multi_child_render_object_mix_in._children;                                           \
    }                                                                                                 \
    inline Array<SlotStorage<__CHILD, __SLOT_DATA>>& children() SKR_NOEXCEPT                          \
    {                                                                                                 \
        return _multi_child_render_object_mix_in._children;                                           \
    }                                                                                                 \
    inline bool need_flush_updates() const SKR_NOEXCEPT                                               \
    {                                                                                                 \
        return _multi_child_render_object_mix_in._need_flush_updates;                                 \
    }                                                                                                 \
    /*===============> End Multi Child Render Object Mixin <===============*/

sreflect_struct(
    "guid": "24fc89ae-b88f-4774-ba8b-9a67fbbb64ee"
)
SKR_GUI_API ISingleChildRenderObject SKR_GUI_INTERFACE_BASE {
    virtual ~ISingleChildRenderObject() = default;

    virtual SKR_GUI_TYPE_ID accept_child_type() const SKR_NOEXCEPT               = 0;
    virtual void            set_child(NotNull<RenderObject*> child) SKR_NOEXCEPT = 0;
    virtual void            remove_child() SKR_NOEXCEPT                          = 0;
};

template <typename TSelf, typename TChild>
struct SingleChildRenderObjectMixin {
    TChild* _child;

    inline SKR_GUI_TYPE_ID accept_child_type(const TSelf& self) const SKR_NOEXCEPT
    {
        return SKR_GUI_TYPE_ID_OF_STATIC(TChild);
    }
    inline void set_child(TSelf& self, NotNull<RenderObject*> child) SKR_NOEXCEPT
    {
        if (_child) _child->unmount();
        _child = child->type_cast_fast<TChild>();
        _child->mount(make_not_null(&self));
    }
    inline void remove_child(TSelf& self) SKR_NOEXCEPT
    {
        if (_child) _child->unmount();
        _child = nullptr;
    }
    inline void visit_children(const TSelf& self, RenderObject::VisitFuncRef visitor) const SKR_NOEXCEPT
    {
        if (_child) visitor(make_not_null(_child));
    }
};

#define SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(__SELF, __CHILD)                         \
    /*===============> Begin Single Child Render Object Mixin <===============*/          \
private:                                                                                  \
    SingleChildRenderObjectMixin<__SELF, __CHILD> _single_child_render_object_mixin = {}; \
                                                                                          \
public:                                                                                   \
    SKR_GUI_TYPE_ID accept_child_type() const SKR_NOEXCEPT override                       \
    {                                                                                     \
        return _single_child_render_object_mixin.accept_child_type(*this);                \
    }                                                                                     \
    void set_child(NotNull<RenderObject*> child) SKR_NOEXCEPT override                    \
    {                                                                                     \
        _single_child_render_object_mixin.set_child(*this, child);                        \
    }                                                                                     \
    void remove_child() SKR_NOEXCEPT override                                             \
    {                                                                                     \
        _single_child_render_object_mixin.remove_child(*this);                            \
    }                                                                                     \
    void visit_children(VisitFuncRef visitor) const SKR_NOEXCEPT override                 \
    {                                                                                     \
        _single_child_render_object_mixin.visit_children(*this, visitor);                 \
    }                                                                                     \
    inline __CHILD* child() const SKR_NOEXCEPT                                            \
    {                                                                                     \
        return _single_child_render_object_mixin._child;                                  \
    }                                                                                     \
    /*===============> End Single Child Render Object Mixin <===============*/

} // namespace gui sreflect
} // namespace skr sreflect

/**********************************render objects**********************************/
namespace skr sreflect
{
namespace gui sreflect
{
sreflect_enum_class(
    "guid" : "f5f07ec1-d733-48ac-89a2-e60961ec5995",
)
ERenderObjectLifecycle : uint8_t{
    Initial,
    Mounted,
    Unmounted,
    Destroyed,
};

sreflect_struct(
    "guid": "6841f5bc-2d9d-4338-b09e-8fbc20bdbf14"
)
SKR_GUI_API RenderObject SKR_GUI_OBJECT_BASE {
    SKR_GUI_OBJECT_ROOT(RenderObject, "74844fa6-8994-4915-8f8e-ec944a1cbea4");
    SKR_GUI_RAII_MIX_IN()
    friend struct PipelineOwner;
    using VisitFuncRef = FunctionRef<void(NotNull<RenderObject*>)>;

    RenderObject() SKR_NOEXCEPT;
    virtual ~RenderObject();

    // lifecycle & tree
    // ctor -> mount <-> unmount -> destroy
    void                          mount(NotNull<RenderObject*> parent) SKR_NOEXCEPT;  // 挂到父节点下
    void                          unmount() SKR_NOEXCEPT;                             // 从父节点卸载
    virtual void                  destroy() SKR_NOEXCEPT;                             // 销毁，生命周期结束
    virtual void                  attach(NotNull<PipelineOwner*> owner) SKR_NOEXCEPT; // 自身或子树挂载到带有 pipeline owner 的树下
    virtual void                  detach() SKR_NOEXCEPT;                              // 自身或子树从带有 pipeline owner 的树下卸载
    inline ERenderObjectLifecycle lifecycle() const SKR_NOEXCEPT { return _lifecycle; }
    inline PipelineOwner*         owner() const SKR_NOEXCEPT { return _owner; }
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
    virtual bool    paints_child(NotNull<RenderObject*> child) const SKR_NOEXCEPT;
    virtual void    apply_paint_transform(NotNull<RenderObject*> child, Matrix4& transform) const SKR_NOEXCEPT;
    virtual Matrix4 get_transform_to(RenderObject* ancestor) const SKR_NOEXCEPT;

    // TODO
    // invoke_layout_callback：用于在 layout 过程中创建 child，通常用于 Sliver
    // layer：repaint_boundary 存储对应 layer 用于局部重绘
    // _paint_with_context：call by PaintingContext
    // handle_event：处理输入事件
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
    RenderObject*  _parent = nullptr;
    PipelineOwner* _owner  = nullptr;
    int32_t        _depth  = 0;

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

struct HitTestRecord {
};

sreflect_struct(
    "guid": "9d12e7d3-fa2e-46da-b035-fab1b3702832"
)
SKR_GUI_API RenderBox : public RenderObject {
    RenderBox();
    ~RenderBox();

    // getter & setter
    inline Sizef          size() const SKR_NOEXCEPT { return _size; }
    inline void           set_size(Sizef size) SKR_NOEXCEPT { _size = size; }
    inline BoxConstraints constraints() const SKR_NOEXCEPT { return _constraints; }
    inline void           set_constraints(BoxConstraints constraints) SKR_NOEXCEPT
    {
        if (_constraints != constraints)
        {
            _constraints = constraints;
            _set_force_relayout_boundary(_constraints.is_tight());
            _set_constraints_changed(true);
        }
    }

    // intrinsic size
    float get_min_intrinsic_width(float height) const SKR_NOEXCEPT;
    float get_max_intrinsic_width(float height) const SKR_NOEXCEPT;
    float get_min_intrinsic_height(float width) const SKR_NOEXCEPT;
    float get_max_intrinsic_height(float width) const SKR_NOEXCEPT;

    // dry layout
    Sizef get_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT;

    // TODO.
    // global_to_local
    // local_to_global
    // paint_bounds
    // hit_test
    // handle_event

protected:
    // intrinsic size
    virtual float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT;
    virtual float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT;
    virtual float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT;
    virtual float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT;

    // dry layout
    virtual Sizef compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT;

private:
    void perform_resize() SKR_NOEXCEPT override; // override compute_dry_layout instead

private:
    Sizef          _size        = {};
    BoxConstraints _constraints = {};

    // TODO. cached data
};

// 代理 Box，其渲染 Sizef 等属性严格由 child 决定，通常起到修饰作用
sreflect_struct(
    "guid": "cf5ab365-abba-4ed8-b7e3-0246edc9cc68"
)
RenderProxyBox : public RenderBox,
                 public ISingleChildRenderObject {
protected:
    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // dry layout
    Sizef compute_dry_layout(BoxConstraints constraints) const SKR_NOEXCEPT override;

    // layout
    void perform_layout() SKR_NOEXCEPT override;

    // paint
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

    // MIXIN
    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderProxyBox, RenderBox);
};

// 会对 child 施加布局偏移的 RenderBox
sreflect_struct(
    "guid": "cc00ea53-4b7c-42de-afc5-287a401b4304"
)
RenderShiftedBox : public RenderBox,
                   public ISingleChildRenderObject {
    inline Offsetf offset() const SKR_NOEXCEPT { return _offset; }
    inline void    set_offset(Offsetf offset) SKR_NOEXCEPT { _offset = offset; }

protected:
    // intrinsic size
    float compute_min_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_width(float height) const SKR_NOEXCEPT override;
    float compute_min_intrinsic_height(float width) const SKR_NOEXCEPT override;
    float compute_max_intrinsic_height(float width) const SKR_NOEXCEPT override;

    // paint
    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;

private:
    Offsetf _offset = {};

    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderShiftedBox, RenderBox)
};

// 概念性的 Window，并不一定是 Root，Root 通常是 RenderNativeWindow
sreflect_struct(
    "guid": "7205c934-bc38-4d09-8ac6-9a18f3f86f68"
)
SKR_GUI_API RenderWindow : public RenderObject,
                           public ISingleChildRenderObject {
    RenderWindow(IWindow* window);

    void paint(NotNull<PaintingContext*> context, Offsetf offset) SKR_NOEXCEPT override;
    void perform_layout() SKR_NOEXCEPT override;

    NotNull<OffsetLayer*> update_layer(OffsetLayer* old_layer) override;

    // getter
    inline IWindow* window() const SKR_NOEXCEPT { return _window; }

private:
    IWindow* _window = nullptr;

    SKR_GUI_SINGLE_CHILD_RENDER_OBJECT_MIXIN(RenderWindow, RenderBox)
};

sreflect_struct(
    "guid": "9540a35d-2c2d-43fc-a37c-64d0a770d97f"
)
SKR_GUI_API RenderNativeWindow : public RenderWindow {
    RenderNativeWindow(INativeWindow* native_window);

    NotNull<OffsetLayer*> update_layer(OffsetLayer* old_layer) override;

    void        prepare_initial_frame() SKR_NOEXCEPT;
    inline void setup_owner(PipelineOwner* owner) SKR_NOEXCEPT { _owner = owner; }
};

} // namespace gui sreflect
} // namespace skr sreflect