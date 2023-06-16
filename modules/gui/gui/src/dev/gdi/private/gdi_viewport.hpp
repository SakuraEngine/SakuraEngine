#pragma once
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gdi
{
struct GDIViewportPrivate : public GDIViewport {
    virtual void add_canvas(GDICanvas* canvas) SKR_NOEXCEPT
    {
        all_canvas_.emplace_back(canvas);
    }
    virtual void remove_canvas(GDICanvas* canvas) SKR_NOEXCEPT
    {
        auto it = eastl::find(all_canvas_.begin(), all_canvas_.end(), canvas);
        if (it != all_canvas_.end())
        {
            all_canvas_.erase(it);
        }
    }
    virtual void clear_canvas() SKR_NOEXCEPT
    {
        all_canvas_.clear();
    }
    virtual Span<GDICanvas*> all_canvas() SKR_NOEXCEPT
    {
        return { all_canvas_.data(), all_canvas_.size() };
    }

    skr::vector<GDICanvas*> all_canvas_;
};
} // namespace skr::gdi