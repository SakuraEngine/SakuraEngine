#pragma once
#include "SkrGui/dev/gdi/gdi_types.hpp"

namespace skr::gdi
{
struct SKR_GUI_API GDICanvas {
    virtual ~GDICanvas() SKR_NOEXCEPT = default;

    virtual void              add_element(GDIElement* element) SKR_NOEXCEPT = 0;
    virtual void              remove_element(GDIElement* element) SKR_NOEXCEPT = 0;
    virtual Span<GDIElement*> all_elements() SKR_NOEXCEPT = 0;
    virtual void              clear_elements() SKR_NOEXCEPT = 0;

    virtual void set_zrange(int32_t min, int32_t max) SKR_NOEXCEPT = 0;
    virtual void get_zrange(int32_t* out_min, int32_t* out_max) SKR_NOEXCEPT = 0;

    virtual void enable_hardware_z() SKR_NOEXCEPT = 0;
    virtual void disable_hardware_z() SKR_NOEXCEPT = 0;
    virtual bool is_hardware_z_enabled() const SKR_NOEXCEPT = 0;

    virtual void set_pivot(float x, float y) SKR_NOEXCEPT = 0;
    virtual void get_pivot(float* out_x, float* out_y) SKR_NOEXCEPT = 0;

    virtual void set_size(float w, float h) SKR_NOEXCEPT = 0;
    virtual void get_size(float* out_w, float* out_h) SKR_NOEXCEPT = 0;
};

} // namespace skr::gdi