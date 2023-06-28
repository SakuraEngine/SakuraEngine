#pragma once
#include "SkrGui/fwd_config.hpp"

namespace skr::gui
{
struct ICanvasService;
struct ITextService;
struct IUpdatableImageEntry;
struct IFileResource;

struct SKR_GUI_API IEmbeddedTextServiceResourceProvider SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IEmbeddedTextServiceResourceProvider, "4649e1d8-e2ea-4a33-a387-b17a5a9f5be4")
    virtual ~IEmbeddedTextServiceResourceProvider() = default;

    virtual NotNull<IUpdatableImageEntry*> create_updatable_image_entry() = 0;
    virtual void                           destroy_updatable_image_entry(NotNull<IUpdatableImageEntry*> entry) = 0;
    virtual Array<uint8_t>                 read_font_file(StringView path) = 0;
};

SKR_GUI_API NotNull<ICanvasService*> create_embedded_canvas_service();
SKR_GUI_API NotNull<ITextService*> create_embedded_text_service(NotNull<IEmbeddedTextServiceResourceProvider*> resource_provider);
SKR_GUI_API void                   destroy_embedded_canvas_service(NotNull<ICanvasService*> service);
SKR_GUI_API void                   destroy_embedded_text_service(NotNull<ITextService*> service);
} // namespace skr::gui