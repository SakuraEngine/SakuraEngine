#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct SKR_GUI_API IGDIDevice {
    virtual ~IGDIDevice() SKR_NOEXCEPT = default;

    [[nodiscard]] static IGDIDevice* Create(EGDIBackend backend);
    static void                      Free(IGDIDevice* device);

    [[nodiscard]] virtual IGDICanvas* create_canvas();
    virtual void                      free_canvas(IGDICanvas* canvas);

    [[nodiscard]] virtual IGDIViewport* create_viewport();
    virtual void                        free_viewport(IGDIViewport* render_group);

    [[nodiscard]] virtual IGDIElement* create_element() = 0;
    virtual void                       free_element(IGDIElement* element) = 0;

    [[nodiscard]] virtual IGDIPaint* create_paint() = 0;
    virtual void                     free_paint(IGDIPaint* paint) = 0;
};

} // namespace skr::gdi