#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct ICanvasService;
struct ITextService;
struct IResourceService;

SKR_GUI_API NotNull<ICanvasService*> create_embedded_canvas_service();
SKR_GUI_API NotNull<ITextService*> create_embedded_text_service(NotNull<IResourceService*> resource_service);
SKR_GUI_API void                   destroy_embedded_canvas_service(NotNull<ICanvasService*> service);
SKR_GUI_API void                   destroy_embedded_text_service(NotNull<ITextService*> service);
} // namespace skr::gui