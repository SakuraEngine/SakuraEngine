#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct IResource;
struct IResourceProvider;
using VisitResourceFuncRef = FunctionRef<void(NotNull<IResource*>)>;

struct SKR_GUI_API IResourceEntry SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResourceEntry, "2664f750-8c30-4f0b-b5d8-4161f0744312")
    virtual ~IResourceEntry() = default;

    virtual NotNull<IResourceProvider*> provider() const SKR_NOEXCEPT = 0;
    virtual void                        destroy_resoure(NotNull<IResource*> resource) SKR_NOEXCEPT = 0;
    virtual void                        visit_requested_resources(VisitResourceFuncRef visitor) = 0;
};
} // namespace skr::gui