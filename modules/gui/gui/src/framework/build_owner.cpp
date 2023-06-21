#include "SkrGui/framework/build_owner.hpp"
#include "misc/defer.hpp"
#include "SkrGui/framework/element/element.hpp"

namespace skr::gui
{
void BuildOwner::schedule_build_for(NotNull<Element*> element) SKR_NOEXCEPT
{
}
void BuildOwner::build_scope(NotNull<Element*> element) SKR_NOEXCEPT
{
}
void BuildOwner::deactivate_element(NotNull<Element*> element) SKR_NOEXCEPT
{
}
} // namespace skr::gui
