#pragma once
#include "SkrGui/gdi.h"
#include <containers/vector.hpp>

namespace skr {
namespace gdi {
struct SKR_GUI_API SGDIElementPrivate : public SGDIElement
{
    virtual ~SGDIElementPrivate() SKR_NOEXCEPT = default;
    
    void set_z(int32_t _z) final
    {
        z = _z;
    }

    virtual int32_t get_z() const
    {
        return z;
    }

    int32_t z = 0.f;
    skr::vector<SGDIVertex> vertices;
    skr::vector<index_t> indices;
    skr::vector<SGDIElementDrawCommand> commands;
};

struct SKR_GUI_API SGDIPaintPrivate : public SGDIPaint
{
    
};

struct SKR_GUI_API SGDICanvasPrivate : public SGDICanvas
{
    virtual void add_element(SGDIElement* element) SKR_NOEXCEPT;
    virtual void remove_element(SGDIElement* element) SKR_NOEXCEPT;
    virtual LiteSpan<SGDIElement*> all_elements() SKR_NOEXCEPT;
    
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

    bool hardware_z_enabled = true;
    int32_t z_min = 0;
    int32_t z_max = 100;
    skr::vector<SGDIElement*> all_elements_;
};

struct SKR_GUI_API SGDICanvasGroupPrivate : public SGDICanvasGroup
{
    virtual void add_canvas(SGDICanvas* canvas) SKR_NOEXCEPT;
    virtual void remove_canvas(SGDICanvas* canvas) SKR_NOEXCEPT;
    virtual LiteSpan<SGDICanvas*> all_canvas() SKR_NOEXCEPT;

    skr::vector<SGDICanvas*> all_canvas_;
};

struct SKR_GUI_API SGDIDevicePrivate : public SGDIDevice
{

};

} }