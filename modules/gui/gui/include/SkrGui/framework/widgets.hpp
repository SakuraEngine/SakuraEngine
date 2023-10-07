#pragma once
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/widget_misc.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/widgets.generated.h"
#endif

namespace skr sreflect
{
namespace gui sreflect
{
sreflect_struct(
    "guid" : "300bf2c3-5e35-47ba-aef5-beae0ce3c992",
)
SKR_GUI_API Widget
{
    // build callback
    virtual void pre_construct() SKR_NOEXCEPT {}
    virtual void post_construct() SKR_NOEXCEPT {}

    // bind element
    virtual NotNull<Element*> create_element() SKR_NOEXCEPT = 0;

    // help function
    static bool can_update(NotNull<Widget*> old_widget, NotNull<Widget*> new_widget) SKR_NOEXCEPT;

    Key key = {};
};

/**********************************basic category**********************************/
sreflect_struct(
    "guid": "1345c1b5-98d4-416b-979e-559587cd8c3d",
)
SKR_GUI_API StatelessWidget : public Widget{};

sreflect_struct(
    "guid": "693bb15f-3405-4513-9b8d-6da144a56310",    
)
SKR_GUI_API StatefulWidget : public Widget{};

sreflect_struct(
    "guid": "fae22ecc-5779-4a77-8357-3e5af7a16bd6"
)
SKR_GUI_API ProxyWidget : public Widget {

    Widget* child = nullptr;
};

sreflect_struct(
    "guid": "5410123c-3851-4af4-a37e-39f993a137c4"
)
SKR_GUI_API RenderObjectWidget : public Widget {
    virtual NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT                                                                      = 0;
    virtual void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT = 0;

    // call after render object detached from render object tree
    virtual void did_unmount_render_object(NotNull<RenderObject*> render_object) SKR_NOEXCEPT;
};

/**********************************ProxyWidget sub category**********************************/
sreflect_struct(
    "guid": "e98681f8-ae7a-4e79-82b6-3c80cc3dd51f",
)
SKR_GUI_API SlotWidget : public ProxyWidget
{
    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    virtual void apply_slot_data(NotNull<RenderObject*> parent, NotNull<RenderObject*> child) const SKR_NOEXCEPT = 0;
};

/**********************************RenderObjectWidget sub category**********************************/
sreflect_struct(
    "guid": "e9227443-0db5-42c1-a978-80d6be99fdc8"
)
SKR_GUI_API LeafRenderObjectWidget : public RenderObjectWidget {
    NotNull<Element*> create_element() SKR_NOEXCEPT override;
};

sreflect_struct(
    "guid": "b2133f0a-b1c9-457f-8bbc-42504ae8850a"
)
SKR_GUI_API SingleChildRenderObjectWidget : public RenderObjectWidget {
    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    Widget* child = nullptr;
};

sreflect_struct(
    "guid": "cf6a2f78-c106-4110-b60e-fe33a6d112f0"
)
SKR_GUI_API MultiChildRenderObjectWidget : public RenderObjectWidget {
    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    Array<Widget*> children = {};
};

sreflect_struct(
    "guid": "1784942d-7945-4bd3-8f0b-a719c3f5ba2b"
)
SKR_GUI_API RenderWindowWidget : public RenderObjectWidget {
    Widget* child = nullptr;
};

sreflect_struct(
    "guid": "986db02c-e9f3-4c71-b004-c165f872bb68"
)
SKR_GUI_API RenderNativeWindowWidget : public RenderWindowWidget {
    NotNull<Element*>      create_element() SKR_NOEXCEPT override;
    NotNull<RenderObject*> create_render_object() SKR_NOEXCEPT override;
    void                   update_render_object(NotNull<IBuildContext*> context, NotNull<RenderObject*> render_object) SKR_NOEXCEPT override;

    RenderNativeWindow* native_window_render_object = nullptr;
};



} // namespace gui sreflect
} // namespace skr sreflect