#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"
#include "SkrGui/framework/widget/stateless_widget.generated.h"

namespace skr::gui
{
struct [[sattr(guid = "8e344044-7b4b-4a69-b7f4-a41672f2c346"
)]] SKR_GUI_API StatelessWidget : public Widget
{
    SKR_GENERATE_BODY(StatelessWidget)

    NotNull<Element*> create_element() SKR_NOEXCEPT override;

    virtual NotNull<Widget*> build(NotNull<IBuildContext*> context) SKR_NOEXCEPT = 0;
};
} // namespace skr::gui