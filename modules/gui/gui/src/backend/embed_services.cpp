#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/backend/canvas/canvas_service.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"
#include "SkrGui/backend/text/text_service.hpp"
#include "backend/embedded_text/text_service.hpp"

namespace skr::gui
{
struct _EmbeddedCanvasService : public ICanvasService {
    SKR_GUI_INTERFACE(_EmbeddedCanvasService, "ced19009-748b-448d-aaa2-ecd11274bb55", ICanvasService)

    NotNull<ICanvas*> create_canvas() override
    {
        return make_not_null(SkrNew<ICanvas>());
    }
    void destroy_canvas(NotNull<ICanvas*> canvas) override
    {
        SkrDelete(canvas.get());
    }
};

NotNull<ICanvasService*> create_embedded_canvas_service()
{
    return make_not_null(SkrNew<_EmbeddedCanvasService>());
}
NotNull<ITextService*> create_embedded_text_service(NotNull<IResourceService*> resource_service)
{
    return make_not_null(SkrNew<_EmbeddedTextService>(resource_service));
}
void destroy_embedded_canvas_service(NotNull<ICanvasService*> service)
{
    SkrDelete(service.get());
}
void destroy_embedded_text_service(NotNull<ITextService*> service)
{
    SkrDelete(service.get());
}
} // namespace skr::gui