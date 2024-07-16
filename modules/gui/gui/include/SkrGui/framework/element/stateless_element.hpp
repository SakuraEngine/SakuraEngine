#pragma once
#include "SkrGui/framework/element/component_element.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#ifndef __meta__
    #include "SkrGui/framework/element/stateless_element.generated.h"
#endif

namespace skr::gui
{
sreflect_struct(
    "guid": "05699161-383d-481e-abfa-ce0a7110dc2c"
)
SKR_GUI_API StatelessElement : public ComponentElement {
    SKR_GENERATE_BODY()
    using Super = ComponentElement;
    using Super::Super;

    // build & update
    Widget* build() SKR_NOEXCEPT override;
    void    update(NotNull<Widget*> new_widget) SKR_NOEXCEPT override;
};
} // namespace skr::gui