#include "SkrGui/framework/build_owner.hpp"
#include "SkrGui/framework/element/element.hpp"

namespace skr::gui
{
void BuildOwner::schedule_build_for(NotNull<Element*> element) SKR_NOEXCEPT
{
    _dirty_elements.emplace_back(element);
}
void BuildOwner::flush_build() SKR_NOEXCEPT
{
    if (_dirty_elements.size() == 0) return;

    // sort by depth and is_dirty
    std::sort(
        _dirty_elements.begin(),
        _dirty_elements.end(),
        +[](Element* a, Element* b) {
            return a->depth() == b->depth() ? a->is_dirty() < b->is_dirty() : a->depth() < b->depth();
        });

    // build
    for (auto element : _dirty_elements)
    {
        element->rebuild();
    }
}
} // namespace skr::gui
