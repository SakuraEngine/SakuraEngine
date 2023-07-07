#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct ICanvas;
struct IParagraph;
struct INativeDevice;

// canvas
SKR_GUI_API NotNull<ICanvas*> embedded_create_canvas() SKR_NOEXCEPT;
SKR_GUI_API void              embedded_destroy_canvas(NotNull<ICanvas*> canvas) SKR_NOEXCEPT;

// text
SKR_GUI_API void embedded_init_text_service(INativeDevice* native_device);
SKR_GUI_API NotNull<IParagraph*> embedded_create_paragraph();
SKR_GUI_API void                 embedded_destroy_paragraph(NotNull<IParagraph*> paragraph);
SKR_GUI_API void                 embedded_shutdown_text_service();
} // namespace skr::gui