#pragma once
#include "SkrGui/framework/diagnostics.hpp"
#include "SkrGui/framework/fwd_framework.hpp"

namespace skr::gui
{

class SKR_GUI_API FocusManager : public DiagnosticableTree
{
    SKR_GUI_OBJECT(FocusManager, "9f9451fd-591f-48c3-a0b7-b08e35eaeb57", DiagnosticableTree)
};

} // namespace skr::gui