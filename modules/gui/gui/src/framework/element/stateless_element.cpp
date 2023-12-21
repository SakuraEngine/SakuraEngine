#include "SkrGui/framework/element/stateless_element.hpp" // IWYU pragma: keep
#include "SkrGui/framework/widget/stateless_widget.hpp"

namespace skr::gui
{
// build & update
Widget* StatelessElement::build() SKR_NOEXCEPT
{
    auto stateless_widget = widget()->type_cast_fast<StatelessWidget>();
    return stateless_widget->build(this).get();
}
void StatelessElement::update(NotNull<Widget*> new_widget) SKR_NOEXCEPT
{
    Super::update(new_widget);
    rebuild(true);
}
} // namespace skr::gui