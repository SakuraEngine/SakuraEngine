#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct SKR_GUI_API GDIDevice {
    virtual ~GDIDevice() SKR_NOEXCEPT = default;

    [[nodiscard]] static GDIDevice* Create(EGDIBackend backend);
    static void                     Free(GDIDevice* device);

    [[nodiscard]] virtual GDICanvas* create_canvas();
    virtual void                     free_canvas(GDICanvas* canvas);

    [[nodiscard]] virtual GDIViewport* create_viewport();
    virtual void                       free_viewport(GDIViewport* render_group);

    [[nodiscard]] virtual GDIElement* create_element() = 0;
    virtual void                      free_element(GDIElement* element) = 0;

    [[nodiscard]] virtual GDIPaint* create_paint() = 0;
    virtual void                    free_paint(GDIPaint* paint) = 0;
};

} // namespace skr::gdi