#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct SKR_GUI_API ITextService SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ITextService, "8de4f30c-6927-4557-a3ad-f38a98c92111")
    virtual ~ITextService() = default;
};
} // namespace skr::gui