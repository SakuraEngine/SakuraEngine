#pragma once
#include "SkrGui/framework/diagnostics.hpp"

namespace skr {
namespace gui {

struct SKR_GUI_API Widget : public DiagnosticableTreeNode
{
    SKR_GUI_TYPE(Widget, DiagnosticableTreeNode, u8"9f69910d-ba18-4ff4-bf5f-3966507c56ba");

};

} }