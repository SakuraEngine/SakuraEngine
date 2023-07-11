#include "SkrGui/framework/pipeline_owner.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"
#include "SkrGui/framework/painting_context.hpp"

namespace skr::gui
{
PipelineOwner::PipelineOwner(INativeDevice* native_device) SKR_NOEXCEPT
    : _native_device(native_device)
{
}

// schedule
void PipelineOwner::schedule_layout_for(NotNull<RenderObject*> node) SKR_NOEXCEPT
{
    _nodes_needing_layout.emplace_back(node);
}
void PipelineOwner::schedule_paint_for(NotNull<RenderObject*> node) SKR_NOEXCEPT
{
    _nodes_needing_paint.emplace_back(node);
}

// flush
void PipelineOwner::flush_layout()
{
    std::sort(
    _nodes_needing_layout.begin(),
    _nodes_needing_layout.end(),
    +[](RenderObject* a, RenderObject* b) {
        return a->depth() < b->depth();
    });

    for (auto node : _nodes_needing_layout)
    {
        if (node->needs_layout() && node->owner() == this)
        {
            node->perform_layout();
            node->_needs_layout = false;
            node->mark_needs_paint();
        }
    }
}
void PipelineOwner::flush_paint()
{
    std::sort(
    _nodes_needing_paint.begin(),
    _nodes_needing_paint.end(),
    +[](RenderObject* a, RenderObject* b) {
        return a->depth() < b->depth();
    });

    for (auto node : _nodes_needing_paint)
    {
        if (node->needs_paint())
        {
            PaintingContext::repaint_composited_child(make_not_null(node));
        }
        else
        {
            PaintingContext::update_layer_properties(make_not_null(node));
        }
    }
}
} // namespace skr::gui