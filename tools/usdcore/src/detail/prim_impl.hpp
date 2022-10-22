#pragma once
#include "EASTL/vector.h"
#include "helpers.hpp"
#include "UsdCore/prim.hpp"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/xform.h"

namespace skd
{
struct SUSDPrimImpl : public USDWrapperWithRC<SUSDPrim>
{
    SUSDPrimImpl(pxr::UsdPrim prim);
    ~SUSDPrimImpl();

    virtual bool GetLocalTransformation(skr::span<double, 16> result, bool* resetsXformStack = nullptr) const override;
        
    // returns false when the prim is not an xformable
    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result) const override;
    virtual void GetLocalToWorldTransformation(skr::span<double, 16> result, double time) const override;

    virtual eastl::vector<SharedId> GetChildren() const SKR_NOEXCEPT override;
    virtual eastl::string GetName() const SKR_NOEXCEPT override;

protected:
    pxr::UsdPrim prim;
};
}