#include "SkrGui/framework/pipeline_owner.hpp"
#include "SkrGui/framework/render_object/render_object.hpp"

namespace skr::gui
{
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
        // TODO. after paint context & layer impl
    }
}
} // namespace skr::gui