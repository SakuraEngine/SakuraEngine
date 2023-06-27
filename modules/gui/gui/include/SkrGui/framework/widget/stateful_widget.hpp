#pragma once
#include "SkrGui/framework/widget/widget.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{
struct SKR_GUI_API StatefulWidget : public Widget {
    SKR_GUI_OBJECT(StatefulWidget, "51c91251-f45e-445e-8caa-0e49b60765cc", Widget);
};
} // namespace skr::gui