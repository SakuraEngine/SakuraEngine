#pragma once
#include "SkrGui/gdi/gdi.hpp"
#include <containers/vector.hpp>

namespace skr {
namespace gdi {
struct SKR_GUI_API GDIElementPrivate : public GDIElement
{
    virtual ~GDIElementPrivate() SKR_NOEXCEPT = default;
    
    void set_z(int32_t _z) final
    {
        z = _z;
    }

    virtual int32_t get_z() const
    {
        return z;
    }

    int32_t z = 0.f;
    skr::vector<GDIVertex> vertices;
    skr::vector<index_t> indices;
    skr::vector<GDIElementDrawCommand> commands;
};

struct SKR_GUI_API GDIPaintPrivate : public GDIPaint
{
    
};

struct SKR_GUI_API GDICanvasPrivate : public GDICanvas
{
    virtual void add_element(GDIElement* element) SKR_NOEXCEPT;
    virtual void remove_element(GDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<GDIElement*> all_elements() SKR_NOEXCEPT;
    virtual void clear_elements() SKR_NOEXCEPT;
    
    virtual void set_zrange(int32_t min, int32_t max) SKR_NOEXCEPT
    {
        z_min = min;
        z_max = max;
    }
    virtual void get_zrange(int32_t* out_min, int32_t* out_max) SKR_NOEXCEPT
    {
        if (out_min) *out_min = z_min;
        if (out_max) *out_max = z_max;
    }

    virtual void enable_hardware_z() SKR_NOEXCEPT 
    {
        hardware_z_enabled = true;
    }
    virtual void disable_hardware_z() SKR_NOEXCEPT 
    {
        hardware_z_enabled = false;
    }
    virtual bool is_hardware_z_enabled() const SKR_NOEXCEPT
    {
        return hardware_z_enabled;
    }

    virtual void set_pivot(float x, float y) SKR_NOEXCEPT
    {
        pivot.x = x;
        pivot.y = y;
    }
    virtual void get_pivot(float* out_x, float* out_y) SKR_NOEXCEPT
    {
        if (out_x) *out_x = pivot.x;
        if (out_y) *out_y = pivot.y;
    }

    virtual void set_size(float w, float h) SKR_NOEXCEPT
    {
        size.x = w;
        size.y = h;
    }
    virtual void get_size(float* out_w, float* out_h) SKR_NOEXCEPT
    {
        if (out_w) *out_w = size.x;
        if (out_h) *out_h = size.y;
    }

    skr_float2_t pivot = { 0.f, 0.f };
    skr_float2_t size = { 0.f, 0.0f };

    bool hardware_z_enabled = true;
    int32_t z_min = 0;
    int32_t z_max = 100;
    skr::vector<GDIElement*> all_elements_;
};

struct SKR_GUI_API GDIViewportPrivate : public GDIViewport
{
    virtual void add_canvas(GDICanvas* canvas) SKR_NOEXCEPT;
    virtual void remove_canvas(GDICanvas* canvas) SKR_NOEXCEPT;
    virtual void clear_canvas() SKR_NOEXCEPT;
    virtual LiteSpan<GDICanvas*> all_canvas() SKR_NOEXCEPT;

    skr::vector<GDICanvas*> all_canvas_;
};

struct SKR_GUI_API GDIDevicePrivate : public GDIDevice
{

};

} }