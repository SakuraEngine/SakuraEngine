#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API BuildOwner {

    // build
    void schedule_build_for(NotNull<Element*> element) SKR_NOEXCEPT;
    void build_scope(NotNull<Element*> element) SKR_NOEXCEPT;
    void drop_unmount_element(NotNull<Element*> element) SKR_NOEXCEPT;
    // TODO. retake_unmounted_element

    // TODO. focus management
    // TODO. navigation management
    // TODO. global key management

private:
    Array<Element*> _inactive_elements;
    Array<Element*> _dirty_elements;
};
} // namespace skr::gui