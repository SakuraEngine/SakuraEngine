#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/framework/key.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API BuildOwner final {

    // build
    void schedule_build_for(NotNull<Element*> element) SKR_NOEXCEPT;
    void flush_build() SKR_NOEXCEPT;

    // TODO. temporal impl for pass compile, move to global context
    inline void drop_unmount_element(NotNull<Element*> element) SKR_NOEXCEPT {}

private:
    Array<Element*> _dirty_elements;
};
} // namespace skr::gui