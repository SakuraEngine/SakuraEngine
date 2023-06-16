#pragma once
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gui
{
struct GDIPaintPrivate : public IGDIPaint {
};

struct GDIElementPrivate : public IGDIElement {
    virtual ~GDIElementPrivate() SKR_NOEXCEPT = default;

    void set_z(int32_t _z) final
    {
        z = _z;
    }

    virtual int32_t get_z() const
    {
        return z;
    }

    void set_texture_swizzle(Swizzle swizzle) SKR_NOEXCEPT final
    {
        texture_swizzle[0] = static_cast<uint32_t>(swizzle.r) + 1;
        texture_swizzle[1] = static_cast<uint32_t>(swizzle.g) + 1;
        texture_swizzle[2] = static_cast<uint32_t>(swizzle.b) + 1;
        texture_swizzle[3] = static_cast<uint32_t>(swizzle.a) + 1;
    }

    int32_t                            z = 0.f;
    uint32_t                           texture_swizzle[4] = { 0, 0, 0, 0 };
    skr::vector<GDIVertex>             vertices;
    skr::vector<GDIIndex>              indices;
    skr::vector<GDIElementDrawCommand> commands;
};
} // namespace skr::gui