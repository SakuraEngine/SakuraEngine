#include "SkrGui/backend/embed_services.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"

namespace skr::gui
{
// canvas
NotNull<ICanvas*> embedded_create_canvas() SKR_NOEXCEPT
{
    return SkrNew<ICanvas>();
}
void embedded_destroy_canvas(NotNull<ICanvas*> canvas) SKR_NOEXCEPT
{
    SkrDelete(canvas.get());
}
} // namespace skr::gui