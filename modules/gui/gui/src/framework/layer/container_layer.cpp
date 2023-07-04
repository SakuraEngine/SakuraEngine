#include "SkrGui/framework/layer/container_layer.hpp"

namespace skr::gui
{
void ContainerLayer::add_child(NotNull<Layer*> child) SKR_NOEXCEPT
{
    child->mount(make_not_null(this));
    _children.push_back(child);
}
bool ContainerLayer::has_children() const SKR_NOEXCEPT
{
    return !_children.empty();
}
void ContainerLayer::remove_all_children() SKR_NOEXCEPT
{
    for (auto child : _children)
    {
        child->unmount();
    }
    _children.clear();
}
} // namespace skr::gui