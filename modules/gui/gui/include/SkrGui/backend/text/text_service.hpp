#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct IParagraph;
struct SKR_GUI_API ITextService SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(ITextService, "8de4f30c-6927-4557-a3ad-f38a98c92111")
    virtual ~ITextService() = default;

    virtual NotNull<IParagraph*> create_paragraph() = 0;
    virtual void                 destroy_paragraph(NotNull<IParagraph*> paragraph) = 0;
};
} // namespace skr::gui