#pragma once
#include "SkrGui/dev/gdi/gdi.hpp"

namespace skr::gdi
{
struct GDIPaintPrivate : public GDIPaint {
};

struct GDIElementPrivate : public GDIElement {
    virtual ~GDIElementPrivate() SKR_NOEXCEPT = default;

    void set_z(int32_t _z) final
    {
        z = _z;
    }

    virtual int32_t get_z() const
    {
        return z;
    }

    void set_texture_swizzle(uint32_t R, uint32_t G, uint32_t B, uint32_t A) SKR_NOEXCEPT final
    {
        texture_swizzle[0] = R;
        texture_swizzle[1] = G;
        texture_swizzle[2] = B;
        texture_swizzle[3] = A;
    }

    int32_t                            z = 0.f;
    uint32_t                           texture_swizzle[4] = { 0, 0, 0, 0 };
    skr::vector<GDIVertex>             vertices;
    skr::vector<GDIIndex>              indices;
    skr::vector<GDIElementDrawCommand> commands;
};
} // namespace skr::gdi