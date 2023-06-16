#pragma once
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gui
{
struct GDICanvasPrivate : public IGDICanvas {
    virtual void add_element(IGDIElement* element) SKR_NOEXCEPT
    {
        all_elements_.emplace_back(element);
    }
    virtual void remove_element(IGDIElement* element) SKR_NOEXCEPT
    {
        auto it = eastl::find(all_elements_.begin(), all_elements_.end(), element);
        if (it != all_elements_.end())
        {
            all_elements_.erase(it);
        }
    }
    virtual Span<IGDIElement*> all_elements() SKR_NOEXCEPT
    {
        return { all_elements_.data(), all_elements_.size() };
    }
    virtual void clear_elements() SKR_NOEXCEPT
    {
        all_elements_.clear();
    }

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

    bool                      hardware_z_enabled = true;
    int32_t                   z_min = 0;
    int32_t                   z_max = 100;
    skr::vector<IGDIElement*> all_elements_;
};

} // namespace skr::gui